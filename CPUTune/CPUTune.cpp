//
//  CPUTune.cpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#include <i386/proc_reg.h>
#include <CPUTune.hpp>
#include <CPUInfo.hpp>
#include <SIPTune.hpp>
#include <kern_util.hpp>
#include <IOKit/IOTimerEventSource.h>
#include <sys/errno.h>

OSDefineMetaClassAndStructors(CPUTune, IOService)

IOService *CPUTune::probe(IOService *provider, SInt32 *score) {
    setProperty("VersionInfo", kextVersion);
    setProperty("Author", "syscl");
    IOService* service = IOService::probe(provider, score);
    return service;
}

bool CPUTune::init(OSDictionary *dict)
{
    LOG("CPUTune(%s) starting on macOS Darwin %d.%d.", kmod_info.version, getKernelVersion(), getKernelMinorVersion());
    if (getKernelVersion() >= KernelVersion::Unsupported && !checkKernelArgument(bootargBeta)) {
        LOG("Unsupported kernel version: %d, get a CPUTune that supports current kernel from https://github.com/syscl/CPUTune", getKernelVersion());
        nvram.setKextPanicKey();
        return false;
    } else if (nvram.isKextPanicLastBoot()) {
        // clear the panic key
        LOG("Found %s key being set in NVRAM, CPUTune(%s) supportted kernel version %d, clear the panic key", kCPUTUNE_PANIC_KEY, kmod_info.version, getKernelVersion());
        nvram.clearKextPanicKey();
    }
    
    bool isDisabled = checkKernelArgument("-s") |
                      checkKernelArgument("-x") |
                      checkKernelArgument(bootargOff);
    if (isDisabled) {
        LOG("not allow to run.");
        return false;
    }
    bool ret = super::init(dict);
    if (!ret) {
        LOG("failed!");
        return ret;
    }
    
    // get string properties
    ProcHotPath = getStringPropertyOrElse("ProcHotAtRuntime", nullptr);
    turboBoostPath = getStringPropertyOrElse("TurboBoostAtRuntime", nullptr);
    speedShiftPath = getStringPropertyOrElse("SpeedShiftAtRuntime", nullptr);
    hwpRequestConfigPath = getStringPropertyOrElse("HWPRequestConfigPath", nullptr);
    turboRatioLimitConfigPath = getStringPropertyOrElse("TurboRatioLimitConfigPath", nullptr);
    // get boolean properties
    enableIntelTurboBoost = getBooleanOrElse("EnableTurboBoost", false);
    enableIntelProcHot = getBooleanOrElse("EnableProcHot", false);
    enableIntelSpeedShift = getBooleanOrElse("EnableSpeedShift", false);
    allowUnrestrictedFS = getBooleanOrElse("AllowUnrestrictedFS", false);
    
    if (OSNumber *timeout = OSDynamicCast(OSNumber, getProperty("UpdateInterval"))) {
        updateInterval = timeout->unsigned32BitValue();
        LOG("Update time interval %u ms per cycle", updateInterval);
    }
    
    LOG("succeeded!");
    
    org_MSR_IA32_MISC_ENABLE = rdmsr64(MSR_IA32_MISC_ENABLE);
    org_MSR_IA32_PERF_CTL = rdmsr64(MSR_IA32_PERF_CTL);
    org_MSR_IA32_POWER_CTL = rdmsr64(MSR_IA32_POWER_CTL);
    if (cpu_info.supportedHWP) {
        // Do not read MSR_IA32_HWP_REQUEST on unsupported HWP's CPU (otherwise will cause panic)
        org_HWPRequest = rdmsr64(MSR_IA32_HWP_REQUEST);
    }
    org_TurboRatioLimit = rdmsr64(MSR_TURBO_RATIO_LIMIT);
    
    return ret;
}

const char* CPUTune::getStringPropertyOrElse(const char* key, const char* defaultProperty) const {
    if (OSString* value = OSDynamicCast(OSString, getProperty(key))) {
        return value->getCStringNoCopy();
    }
    return defaultProperty;
}

const bool CPUTune::getBooleanOrElse(const char* key, const bool defaultValue) const {
    if (OSBoolean* value = OSDynamicCast(OSBoolean, getProperty(key))) {
        return value->isTrue();
    }
    return defaultValue;
}

bool CPUTune::start(IOService *provider)
{
    bool ret = super::start(provider);
    if (!ret || provider == nullptr) {
        LOG("cannot start provider or provider does not exist.");
        return ret;
    }
    
    // let's turn off some of the SIP bits so that we can debug it easily on a real mac
    if (allowUnrestrictedFS) {
        sip_tune.allowUnrestrictedFS();
    }
    
    // set up time event
    myWorkLoop = static_cast<IOWorkLoop *>(getWorkLoop());
    timerSource = IOTimerEventSource::timerEventSource(this,
                    OSMemberFunctionCast(IOTimerEventSource::Action, this, &CPUTune::readConfigAtRuntime));
    if (!timerSource) {
        LOG("failed to create timer event source!");
        // Handle error (typically by returning a failure result).
        return false;
    }
    
    if (myWorkLoop->addEventSource(timerSource) != kIOReturnSuccess) {
        LOG("failed to add timer event source to work loop!");
        // Handle error (typically by returning a failure result).
        return false;
    }
    
    timerSource->setTimeoutMS(updateInterval);
    
    // check if we need to enable Intel Turbo Boost
    if (enableIntelTurboBoost) {
        enableTurboBoost();
    } else {
        disableTurboBoost();
    }

    // make sure we disable ProcHot only if turboboost is disabled
    if (enableIntelProcHot) {
        enableProcHot();
    } else if(!enableIntelTurboBoost) {
        disableProcHot();
    } else {
        LOG("cannot deactivate PROCHOT while turboboost is active!");
    }
    
    // check if we need to enable Intel Speed Shift on platform on Skylake+
    if (cpu_info.supportedHWP) {
        if (!hwpEnableOnceSet && enableIntelSpeedShift) {
            // Note this bit can only be enabled once from the default value.
            // Once set, writes to the HWP_ENABLE bit are ignored. Only RESET
            // will clear this bit. Default = zero (0).
            enableSpeedShift();
            hwpEnableOnceSet = true;
        } 
    } else {
        LOG("cpu model (0x%x) does not support Intel SpeedShift.", cpu_info.model);
    }
    
    LOG("registerService");
    registerService();
    return ret;
}

void CPUTune::readConfigAtRuntime(OSObject *owner, IOTimerEventSource *sender)
{
    // FIXME: As per Apple Document (https://developer.apple.com/library/archive/documentation/DeviceDrivers/Conceptual/IOKitFundamentals/HandlingEvents/HandlingEvents.html#//apple_ref/doc/uid/TP0000018-BAJFFJAD Listing 7-5):
    // Events originating from timers are handled by the driver’s Action routine.
    // As with other event handlers, this routine should never block indefinitely.
    // This specifically means that timer handlers, and any function they invoke,
    // must not allocate memory or create objects, as allocation can block for unbounded periods of time.
    // As for now, the reading procedure reads only one byte, which is fairly fast in our case, so we assume
    // this routine will not cause infinite blocking. Let me know if you have some other good ideas.
    if (turboBoostPath) {
        // check if previous turbo boost is enabled
        bool prev = rdmsr64(MSR_IA32_MISC_ENABLE) == (org_MSR_IA32_MISC_ENABLE & kEnableTurboBoostBits);
        uint8_t *buffer = readFileAsBytes(turboBoostPath, 0, 1);
        // check if currently request enable turbo boost
        bool curr = buffer && (*buffer == '1');
        // deallocate the buffer
        deleter(buffer);
        if (curr != prev) {
            LOG("%s Intel Turbo Boost", curr ? "enable" : "disable");
            if (curr) {
                enableTurboBoost();
            } else {
                disableTurboBoost();

            }
        }
    }
    
    // Turbo ratio limit
    if ((rdmsr64(MSR_IA32_MISC_ENABLE) & kEnableTurboBoostBits) && cpu_info.turboRatioLimitRW && turboRatioLimitConfigPath) {
        size_t valid_length = cpu_info.coreCount * 2 + 2; // +2 for '0x'/'0X'
        if (uint8_t* config = readFileAsBytes(turboRatioLimitConfigPath, 0, valid_length)) {
            long limit = hexToInt(reinterpret_cast<char*>(config));
            deleter(config);
            if (limit == ERANGE) {
                LOG("Turbo ratio limit is not a valid hexadecimal constant at %s", limit, turboRatioLimitConfigPath);
            } else {
                uint64_t curLimit = rdmsr64(MSR_TURBO_RATIO_LIMIT);
                uint64_t usrLimit = static_cast<uint64_t>(limit);
                if (setIfNotEqual(curLimit, usrLimit, MSR_TURBO_RATIO_LIMIT)) {
                    LOG("Change turbo ratio limit: 0x%llx -> 0x%llx", curLimit, usrLimit);
                }
            }
        }
    }

    if (ProcHotPath) {
        bool prev = rdmsr64(MSR_IA32_POWER_CTL) & kEnableProcHotBit;
        uint8_t *buffer = readFileAsBytes(ProcHotPath, 0, 1);
        // check if currently request enable ProcHot
        bool curr = buffer && (*buffer == '1');
        // deallocate the buffer
        deleter(buffer);
        if (curr != prev) {
            LOG("%s Intel Proc Hot", curr ? "enable" : "disable");
            if (curr) {
                enableProcHot();
            } else if((rdmsr64(MSR_IA32_MISC_ENABLE) & kEnableTurboBoostBits) != kEnableTurboBoostBits){
                disableProcHot();
            } else {
                LOG("Cannot disable PROCHOT while turboboost is active!");
            }
                
        }
    }
    
    // set hwp request value if hwp is enable
    if (cpu_info.supportedHWP && hwpRequestConfigPath) {
        if (uint8_t *hex = readFileAsBytes(hwpRequestConfigPath, 0, 10)) {
            // hex is not NULL means the hwp request config exist
            // let's check if the hex is valid before writing to MSR
            long req = hexToInt(reinterpret_cast<char*>(hex));
            deleter(hex);
            if (req == ERANGE) {
                LOG("HWP Request %s is not a valid hexadecimal constant at %s", hex, hwpRequestConfigPath);
            } else {
                uint64_t curHWPRequest = rdmsr64(MSR_IA32_HWP_REQUEST);
                uint64_t usrHWPRequest = static_cast<uint64_t>(req);
                if (setIfNotEqual(curHWPRequest, usrHWPRequest, MSR_IA32_HWP_REQUEST)) {
                    LOG("change MSR_IA32_HWP_REQUEST(0x%llx): 0x%llx -> 0x%llx", MSR_IA32_HWP_REQUEST, curHWPRequest, usrHWPRequest);
                }
            }
        }
    }
    
    if (!hwpEnableOnceSet && cpu_info.supportedHWP && speedShiftPath) {
        // check if previous speed shift is enabled
        bool prev = rdmsr64(MSR_IA32_PM_ENABLE) == kEnableSpeedShiftBit;
        uint8_t *buffer = readFileAsBytes(speedShiftPath, 0, 1);
        // check if currently request enable speed shift
        bool curr = buffer && (*buffer == '1');
        if (buffer && curr != prev) {
            LOG("%s Intel Speed Shift", curr ? "enable" : "disable");
            if (curr) {
                enableSpeedShift();
            } else {
                disableSpeedShift();
            }
        }
        // deallocate the buffer
        deleter(buffer);
    }
    sender->setTimeoutMS(updateInterval);
}

bool CPUTune::setIfNotEqual(const uint64_t current, const uint64_t expect, const uint32_t msr) const {
    bool needWrite = current != expect;
    if (needWrite) {
        wrmsr64(msr, expect);
    }
    return needWrite;
}

void CPUTune::enableTurboBoost()
{
    const uint64_t cur = rdmsr64(MSR_IA32_MISC_ENABLE);
    // flip bit 38 to 0
    const uint64_t val = cur & kEnableTurboBoostBits;
    if (setIfNotEqual(cur, val, MSR_IA32_MISC_ENABLE)) {
        LOG("change 0x%llx to 0x%llx in MSR_IA32_MISC_ENABLE(0x%llx)", cur, val, MSR_IA32_MISC_ENABLE);
    } else {
        LOG("0x%llx in MSR_IA32_MISC_ENABLE(0x%llx) remains the same", cur, MSR_IA32_MISC_ENABLE);
    }
}

void CPUTune::disableTurboBoost()
{
    const uint64_t cur = rdmsr64(MSR_IA32_MISC_ENABLE);
    // flip bit 38 to 1
    const uint64_t val = cur | kDisableTurboBoostBits;
    if (setIfNotEqual(cur, val, MSR_IA32_MISC_ENABLE)) {
        LOG("change 0x%llx to 0x%llx in MSR_IA32_MISC_ENABLE(0x%llx)", cur, val, MSR_IA32_MISC_ENABLE);
    } else {
        LOG("0x%llx in MSR_IA32_MISC_ENABLE(0x%llx) remains the same", cur, MSR_IA32_MISC_ENABLE);
    }
}

void CPUTune::disableProcHot()
{
    const uint64_t cur = rdmsr64(MSR_IA32_POWER_CTL);
    const uint64_t val = cur & kDisableProcHotBit;
    if (setIfNotEqual(cur, val, MSR_IA32_POWER_CTL)) {
        LOG("change 0x%llx to 0x%llx in MSR_IA32_POWERCTL(0x%llx)", cur, val, MSR_IA32_POWER_CTL);
    } else {
        LOG("0x%llx in MSR_IA32_POWER_CTL(0x%llx) remains the same", cur, MSR_IA32_POWER_CTL);
    }
}

void CPUTune::enableProcHot()
{
    const uint64_t cur = rdmsr64(MSR_IA32_POWER_CTL);
    const uint64_t val = cur | kEnableProcHotBit;
    if(setIfNotEqual(cur, val, MSR_IA32_POWER_CTL)) {
        LOG("change 0x%llx to 0x%llx in MSR_IA32_POWERCTL(0x%llx)", cur, val, MSR_IA32_POWER_CTL);
    } else {
        LOG("0x%llx in MSR_IA32_POWER_CTL(0x%llx) remains the same", cur, MSR_IA32_POWER_CTL);
    }
}

void CPUTune::enableSpeedShift()
{
    const uint64_t cur = rdmsr64(MSR_IA32_PM_ENABLE);
    if (setIfNotEqual(cur, kEnableSpeedShiftBit, MSR_IA32_PM_ENABLE)) {
        LOG("change 0x%llx to 0x%llx in MSR_IA32_PM_ENABLE(0x%llx)", cur, kEnableSpeedShiftBit, MSR_IA32_PM_ENABLE);
    } else {
        LOG("0x%llx in MSR_IA32_PM_ENABLE(0x%llx) remains the same", cur, MSR_IA32_PM_ENABLE);
    }
}

void CPUTune::disableSpeedShift()
{
    const uint64_t cur = rdmsr64(MSR_IA32_PM_ENABLE);
    if (setIfNotEqual(cur, kDisableSpeedShiftBit, MSR_IA32_PM_ENABLE)) {
        LOG("change 0x%llx to 0x%llx in MSR_IA32_PM_ENABLE(0x%llx)", cur, kEnableSpeedShiftBit, MSR_IA32_PM_ENABLE);
    } else {
        LOG("0x%llx in MSR_IA32_PM_ENABLE(0x%llx) remains the same", cur, MSR_IA32_PM_ENABLE);
    }
}

void CPUTune::stop(IOService *provider)
{
    // Disposing of a timer event source
    if (timerSource) {
        timerSource->cancelTimeout();
        myWorkLoop->removeEventSource(timerSource);
        timerSource->release();
        timerSource = 0;
    }

    // restore the previous MSR_IA32 state
    const uint64_t cur_ctk = rdmsr64(MSR_IA32_POWER_CTL);
    if (setIfNotEqual(cur_ctk, org_MSR_IA32_POWER_CTL, MSR_IA32_POWER_CTL)) {
        LOG("restore MSR_IA32_POWER_CTK from 0x%llx to 0x%llx", cur_ctk, org_MSR_IA32_POWER_CTL);
    }
    const uint64_t cur_misc = rdmsr64(MSR_IA32_MISC_ENABLE);
    if (setIfNotEqual(cur_misc, org_MSR_IA32_MISC_ENABLE, MSR_IA32_MISC_ENABLE)) {
        LOG("restore MSR_IA32_MISC_ENABLE from 0x%llx to 0x%llx", cur_misc, org_MSR_IA32_MISC_ENABLE);
    }
    const uint64_t cur_perf_ctl = rdmsr64(MSR_IA32_PERF_CTL);
    if (setIfNotEqual(cur_perf_ctl, org_MSR_IA32_PERF_CTL, MSR_IA32_PERF_CTL)) {
        LOG("restore MSR_IA32_PERF_CTL from 0x%llx to 0x%llx", cur_perf_ctl, org_MSR_IA32_PERF_CTL);
    }
    if (cpu_info.supportedHWP) {
        const uint64_t cur_pm_enable = rdmsr64(MSR_IA32_PM_ENABLE);
        if (setIfNotEqual(cur_pm_enable, org_MSR_IA32_PM_ENABLE, MSR_IA32_PM_ENABLE)) {
            LOG("restore MSR_IA32_PM_ENABLE from 0x%llx to 0x%llx", cur_pm_enable, org_MSR_IA32_PM_ENABLE);
        }
        const uint64_t cur_hwp_req = rdmsr64(MSR_IA32_HWP_REQUEST);
        if (setIfNotEqual(cur_hwp_req, org_HWPRequest, MSR_IA32_HWP_REQUEST)) {
            LOG("restore MSR_IA32_HWP_REQUEST(0x%llx) from 0x%llx to 0x%llx", cur_hwp_req, org_HWPRequest);
        }
    }
    super::stop(provider);
}

void CPUTune::free(void)
{
    super::free();
}
