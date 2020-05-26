//
//  CPUInfo.cpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#include "CPUInfo.hpp"
#include <i386/proc_reg.h>

const uint8_t CPUInfo::getCPUModel() const {
    uint32_t cpuid_reg[4];
    do_cpuid(0x00000001, cpuid_reg);
    return bitfield32(cpuid_reg[eax], 7,  4) + (bitfield32(cpuid_reg[eax], 19, 16) << 4);
}

const bool CPUInfo::supportedSpeedShift() const {
    return model >= CPU_MODEL_SKYLAKE;
}

const uint8_t CPUInfo::getCoreCount() const {
    uint64_t coreThreadCountMSR = rdmsr64(MSR_CORE_THREAD_COUNT);
    return bitfield32(coreThreadCountMSR, 31, 16);
}
