//
//  CPUInfo.cpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#include "CPUInfo.hpp"

uint8_t CPUInfo::getCPUModel() const {
    uint32_t cpuid_reg[4];
    do_cpuid(0x00000001, cpuid_reg);
    return bitfield32(cpuid_reg[eax], 7,  4) + (bitfield32(cpuid_reg[eax], 19, 16) << 4);
}
