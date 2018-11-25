//
//  CPUTune.cpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#include <i386/proc_reg.h>
#include <CPUTune.hpp>
#include <CPUInfo.hpp>
#include <kern_util.hpp>
#include <IOKit/IOTimerEventSource.h>

OSDefineMetaClassAndStructors(CPUTune, IOService)

IOService *CPUTune::probe(IOService *provider, SInt32 *score) {
    setProperty("VersionInfo", kextVersion);
    setProperty("Author", "syscl");
    auto service = IOService::probe(provider, score);
    return service;
}

bool CPUTune::init(OSDictionary *dict)
{
    auto isDisabled = checkKernelArgument("-s") |
                      checkKernelArgument("-x") |
                      checkKernelArgument(bootargOff) |
                      (getKernelVersion() >= KernelVersion::Unsupported && !checkKernelArgument(bootargBeta));
    if (isDisabled) {
        myLOG("init: not allow to run.");
        return false;
    }
    bool ret = super::init(dict);
    if (!ret) {
        myLOG("init: failed!");
        return ret;
    }
    cpu_info = new CPUInfo();
    if (cpu_info == nullptr) {
        myLOG("init: cannot allocate class CPUInfo.");
        return false;
    }
    myLOG("init: successed!");
    org_MSR_IA32_MISC_ENABLE = rdmsr64(MSR_IA32_MISC_ENABLE);
    org_MSR_IA32_PERF_CTL = rdmsr64(MSR_IA32_PERF_CTL);
    return ret;
}

void CPUTune::initKextPerferences()
{
    OSString *keyTurboBoostAtRuntime = OSDynamicCast(OSString, getProperty("TurboBoostAtRuntime"));
    OSString *keySpeedShiftAtRuntime = OSDynamicCast(OSString, getProperty("SpeedShiftAtRuntime"));
    OSBoolean *keyEnableTurboBoost = OSDynamicCast(OSBoolean, getProperty("enableTurboBoost"));
    OSBoolean *keyDisableSpeedShift = OSDynamicCast(OSBoolean, getProperty("disableSpeedShift"));
    
    if (keyTurboBoostAtRuntime != nullptr) {
        turboBoostPath = const_cast<const char *>(keyTurboBoostAtRuntime->getCStringNoCopy());
    }
    
    if (keySpeedShiftAtRuntime != nullptr) {
        speedShiftPath = const_cast<const char *>(keySpeedShiftAtRuntime->getCStringNoCopy());
    }

    enableIntelTurboBoost = keyEnableTurboBoost && keyEnableTurboBoost->isTrue();
    supportedSpeedShift = cpu_info->model >= cpu_info->CPU_MODEL_SKYLAKE;
    disableIntelSpeedShift = keyDisableSpeedShift && keyDisableSpeedShift->isTrue();
}

bool CPUTune::start(IOService *provider)
{
    bool ret = super::start(provider);
    if (!ret || provider == nullptr) {
        myLOG("start: cannot start provider or provider does not exist.");
        return ret;
    }
    
    initKextPerferences();
    
    if (turboBoostPath || speedShiftPath) {
        // set up time event
        myWorkLoop = static_cast<IOWorkLoop *>(getWorkLoop());
        timerSource = IOTimerEventSource::timerEventSource(this,
            OSMemberFunctionCast(IOTimerEventSource::Action, this, &CPUTune::readConfigAtRuntime));
        if (!timerSource) {
            myLOG("start: failed to create timer event source!");
            // Handle error (typically by returning a failure result).
            return false;
        }
        
        if (myWorkLoop->addEventSource(timerSource) != kIOReturnSuccess) {
            myLOG("start: failed to add timer event source to work loop!");
            // Handle error (typically by returning a failure result).
            return false;
        }
        
        timerSource->setTimeoutMS(1000);
    }
    
    // check if we need to enable Intel Turbo Boost
    if (enableIntelTurboBoost) {
        enableTurboBoost();
    } else {
        disableTurboBoost();
    }
    
    // check if we need to disable Intel Speed Shift on platform on Skylake+
    if (supportedSpeedShift) {
        if (disableIntelSpeedShift) {
            disableSpeedShift();
        } else {
            enableSpeedShift();
        }
    } else {
        myLOG("start: cpu model (0x%x) does not support Intel SpeedShift.", cpu_info->model);
    }
    
    myLOG("start: registerService");
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
        size_t bytes = 1;
        uint8_t *buffer = readFileNBytes(turboBoostPath, 0, bytes);
        // check if currently request enable turbo boost
        bool curr = buffer && (*buffer == '1');
        // deallocate the buffer
        if (buffer) {
            delete buffer;
            buffer = nullptr;
        }
        if (curr != prev) {
            myLOG("readConfigAtRuntime: %s Intel Turbo Boost", curr ? "enable" : "disable");
            if (curr) {
                enableTurboBoost();
            } else {
                disableTurboBoost();
            }
        }
    }
    
    if (supportedSpeedShift && speedShiftPath) {
        // check if previous speed shift is enabled
        bool prev = rdmsr64(MSR_IA32_PM_ENABLE);
        size_t bytes = 1;
        uint8_t *buffer = readFileNBytes(speedShiftPath, 0, bytes);
        // check if currently request enable speed shift
        bool curr = buffer && (*buffer == '1');
        // deallocate the buffer
        if (buffer) {
            delete buffer;
            buffer = nullptr;
        }
        if (curr != prev) {
            myLOG("readConfigAtRuntime: %s Intel Speed Shift", curr ? "enable" : "disable");
            if (curr) {
                enableSpeedShift();
            } else {
                disableSpeedShift();
            }
        }
    }
    sender->setTimeoutMS(2000);
}

void CPUTune::enableTurboBoost()
{
    const uint64_t cur = rdmsr64(MSR_IA32_MISC_ENABLE);
    // flip bit 38 to 0
    const uint64_t val = cur & kEnableTurboBoostBits;
    myLOG("enableTurboBoost: get MSR_IA32_MISC_ENABLE value: 0x%llx", cur);
    if (val == cur) {
        myLOG("enableTurboBoost: Intel Turbo Boost has already been enabled.");
    } else {
        myLOG("enableTurboBoost: set MSR_IA32_MISC_ENABLE value: 0x%llx", val);
        wrmsr64(MSR_IA32_MISC_ENABLE, val);
    }
}

void CPUTune::disableTurboBoost()
{
//    uint64_t val = org_MSR_IA32_PERF_CTL;
//    myLOG("disableTurboBoost: get MSR_IA32_PERF_CTL value: 0x%llx", val);
//     flip bit 32 to 1
//    val &= ((uint64_t)1) << 32;
//    myLOG("disableTurboBoost: set MSR_IA32_PERF_CTL value: 0x%llx", val);
//    wrmsr64(MSR_IA32_PERF_CTL, val);
    const uint64_t cur = rdmsr64(MSR_IA32_MISC_ENABLE);
    // flip bit 38 to 1
    const uint64_t val = cur | kDisableTurboBoostBits;
    myLOG("disableTurboBoost: get MSR_IA32_MISC_ENABLE value: 0x%llx", cur);
    if (val == cur) {
        myLOG("disableTurboBoost: Intel Turbo Boost has already been disabled.");
    } else {
        myLOG("disableTurboBoost: set MSR_IA32_MISC_ENABLE to 0x%llx", val);
        wrmsr64(MSR_IA32_MISC_ENABLE, val);
    }
}

void CPUTune::enableSpeedShift()
{
    auto val = rdmsr64(MSR_IA32_PM_ENABLE);
    myLOG("enableSpeedShift: get MSR_IA32_PM_ENABLE value: 0x%llx", val);
    if (val == kEnableSpeedShiftBit) {
        myLOG("enableSpeedShift: Intel SpeedShift has already been enabled.");
    } else {
        myLOG("enableSpeedShift: Intel SpeedShift is disabled, turn it on.");
        wrmsr64(MSR_IA32_PM_ENABLE, kEnableSpeedShiftBit);
    }
}

void CPUTune::disableSpeedShift()
{
    auto val = rdmsr64(MSR_IA32_PM_ENABLE);
    myLOG("enableSpeedShift: get MSR_IA32_PM_ENABLE value: 0x%llx", val);
    if (val == kDisableSpeedShiftBit) {
        myLOG("enableSpeedShift: Intel SpeedShift has already been disabled.");
    } else {
        myLOG("enableSpeedShift: Intel SpeedShift is enabled, turn it off.");
        wrmsr64(MSR_IA32_PM_ENABLE, kDisableSpeedShiftBit);
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

    // restore back the previous MSR_IA32 state
    auto cur_MSR_IA32_MISC_ENABLE = rdmsr64(MSR_IA32_MISC_ENABLE);
    auto cur_MSR_IA32_PERF_CTL = rdmsr64(MSR_IA32_PERF_CTL);
    if (cur_MSR_IA32_MISC_ENABLE != org_MSR_IA32_MISC_ENABLE) {
        myLOG("stop: restore MSR_IA32_MISC_ENABLE from 0x%llx to 0x%llx", cur_MSR_IA32_MISC_ENABLE, org_MSR_IA32_MISC_ENABLE);
        wrmsr64(MSR_IA32_MISC_ENABLE, org_MSR_IA32_MISC_ENABLE);
    }
    if (cur_MSR_IA32_PERF_CTL != org_MSR_IA32_PERF_CTL) {
        myLOG("stop: restore MSR_IA32_PERF_CTL from 0x%llx to 0x%llx", cur_MSR_IA32_PERF_CTL, org_MSR_IA32_PERF_CTL);
        wrmsr64(MSR_IA32_PERF_CTL, org_MSR_IA32_PERF_CTL);
    }
    if (cpu_info->model >= cpu_info->CPU_MODEL_SKYLAKE) {
        auto cur_MSR_IA32_PM_ENABLE = rdmsr64(MSR_IA32_PM_ENABLE);
        if (cur_MSR_IA32_PM_ENABLE != org_MSR_IA32_PM_ENABLE) {
            myLOG("stop: restore MSR_IA32_PM_ENABLE from 0x%llx to 0x%llx", cur_MSR_IA32_PM_ENABLE, org_MSR_IA32_PM_ENABLE);
            wrmsr64(MSR_IA32_PM_ENABLE, org_MSR_IA32_PM_ENABLE);
        }
    }
    super::stop(provider);
}

void CPUTune::free(void)
{
    if (cpu_info) {
        delete cpu_info;
        cpu_info = nullptr;
    }
    super::free();
}
