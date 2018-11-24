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
    myLOG("init: successed!");
    org_MSR_IA32_MISC_ENABLE = rdmsr64(MSR_IA32_MISC_ENABLE);
    org_MSR_IA32_PERF_CTL = rdmsr64(MSR_IA32_PERF_CTL);
    return ret;
}

bool CPUTune::start(IOService *provider)
{
    bool ret = super::start(provider);
    if (!ret || provider == nullptr) {
        myLOG("start: cannot start provider or provider not exist.");
        return ret;
    }
    /**
     * Turbo Boost is supported but not enabled (yet) and thus we
     * check the preference to see if we should enable it.
     */
    OSBoolean *key_enableTurboBoost = OSDynamicCast(OSBoolean, getProperty("enableTurboBoost"));
    if (key_enableTurboBoost) {
        // key exists
        auto enableIntelTB = key_enableTurboBoost->isTrue();
        if (enableIntelTB) {
            enableTurboBoost();
        } else {
            disableTurboBoost();
        }
    }
    OSSafeReleaseNULL(key_enableTurboBoost);
    OSBoolean *key_disableSpeedShift = OSDynamicCast(OSBoolean, getProperty("disableSpeedShift"));
    if (key_disableSpeedShift) {
        // key exists
        auto disableSpeedShift = key_disableSpeedShift->isTrue();
        CPUInfo cpu_info = CPUInfo();
        if (cpu_info.cpuModel >= cpu_info.CPU_MODEL_SKYLAKE) {
            if (disableSpeedShift) {
                this->disableSpeedShift();
            } else {
                enableSpeedShift();
            }
        } else {
            myLOG("start: cpu model does not support Intel SpeedShift.");
        }
    }
    OSSafeReleaseNULL(key_disableSpeedShift);
    myLOG("start: registerService");
    registerService();
    return ret;
}

void CPUTune::enableTurboBoost()
{
    uint64_t val = org_MSR_IA32_MISC_ENABLE;
    myLOG("enableTurboBoost: get MSR_IA32_MISC_ENABLE value: 0x%llx", val);
    // flip bit 38 to 0
    val &= ((uint64_t)-1) ^ ((uint64_t)1) << 38;
    myLOG("enableTurboBoost: set MSR_IA32_MISC_ENABLE value: 0x%llx", val);
    wrmsr64(MSR_IA32_MISC_ENABLE, val);
}

void CPUTune::disableTurboBoost()
{
//    uint64_t val = org_MSR_IA32_PERF_CTL;
//    myLOG("disableTurboBoost: get MSR_IA32_PERF_CTL value: 0x%llx", val);
//     flip bit 32 to 1
//    val &= ((uint64_t)1) << 32;
//    myLOG("disableTurboBoost: set MSR_IA32_PERF_CTL value: 0x%llx", val);
//    wrmsr64(MSR_IA32_PERF_CTL, val);
    uint64_t val = org_MSR_IA32_MISC_ENABLE;
    myLOG("disableTurboBoost: get MSR_IA32_MISC_ENABLE value: 0x%llx", val);
    //     flip bit 38 to 1
    val |= ~(((uint64_t)-1) ^ ((uint64_t)1) << 38);
    myLOG("disableTurboBoost: set MSR_IA32_MISC_ENABLE value: 0x%llx", val);
    wrmsr64(MSR_IA32_MISC_ENABLE, val);
}

void CPUTune::enableSpeedShift()
{
    auto val = rdmsr64(MSR_IA32_PM_ENABLE);
    myLOG("enableSpeedShift: get MSR_IA32_PM_ENABLE value: 0x%llx", val);
    if (val != 0x1) {
        myLOG("enableSpeedShift: Intel SpeedShift is disabled, turn it on.");
        wrmsr64(MSR_IA32_PM_ENABLE, 0x1);
    } else {
        myLOG("enableSpeedShift: Intel SpeedShift is already enabled.");
    }
}

void CPUTune::disableSpeedShift()
{
    auto val = rdmsr64(MSR_IA32_PM_ENABLE);
    myLOG("enableSpeedShift: get MSR_IA32_PM_ENABLE value: 0x%llx", val);
    if (val != 0) {
        myLOG("enableSpeedShift: Intel SpeedShift is enabled, turn it off.");
        wrmsr64(MSR_IA32_PM_ENABLE, 0);
    } else {
        myLOG("enableSpeedShift: Intel SpeedShift is already disabled.");
    }
}

void CPUTune::stop(IOService *provider)
{
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
    super::stop(provider);
}

void CPUTune::free(void)
{
    super::free();
}
