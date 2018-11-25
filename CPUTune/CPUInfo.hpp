//
//  CPUInfo.hpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#ifndef CPUInfo_hpp
#define CPUInfo_hpp

#include "kern_util.hpp"
#include <i386/cpuid.h>
/* Copied from xnu/osfmk/cpuid.c (modified for 64-bit values) */
#define bit(n)                 (1UL << (n))
#define bitmask64(h, l)        ((bit(h) | (bit(h) - 1)) & ~ (bit(l) - 1))
#define bitfield32(x, h, l)    (((x) & bitmask64(h, l)) >> l)

// Intel SpeedShift MSRs
#define MSR_IA32_PM_ENABLE          0x770

class CPUInfo {
public:
    CPUInfo() : model(getCPUModel()) {
        myLOG("CPUInfo: cpu model: 0x%x", model);
    };
    
    ~CPUInfo() {
        myLOG("~CPUInfo(): OK!");
    }
    
    /**
     * Intel CPU models
     */
    uint8_t model;
    
    /**
     *  Get current CPU model.
     *
     *  @return detected CPU model
     */
    uint8_t getCPUModel(void) const;
    
    /**
     *  Intel CPU models as returned by CPUID
     *  The list is synchronised and updated with XNU source code (osfmk/i386/cpuid.h).
     *  Last update: xnu-4570.41.2
     *  Some details could be found on http://instlatx64.atw.hu and https://en.wikichip.org/wiki/64-bit_architecture#x86
     */
    enum CpuModel {
        CPU_MODEL_UNKNOWN        =  0x00,
        CPU_MODEL_PENRYN         =  0x17,
        CPU_MODEL_NEHALEM        =  0x1A,
        CPU_MODEL_FIELDS         =  0x1E, /* Lynnfield, Clarksfield */
        CPU_MODEL_DALES          =  0x1F, /* Havendale, Auburndale */
        CPU_MODEL_NEHALEM_EX     =  0x2E,
        CPU_MODEL_DALES_32NM     =  0x25, /* Clarkdale, Arrandale */
        CPU_MODEL_WESTMERE       =  0x2C, /* Gulftown, Westmere-EP/-WS */
        CPU_MODEL_WESTMERE_EX    =  0x2F,
        CPU_MODEL_SANDYBRIDGE    =  0x2A,
        CPU_MODEL_JAKETOWN       =  0x2D,
        CPU_MODEL_IVYBRIDGE      =  0x3A,
        CPU_MODEL_IVYBRIDGE_EP   =  0x3E,
        CPU_MODEL_CRYSTALWELL    =  0x46,
        CPU_MODEL_HASWELL        =  0x3C,
        CPU_MODEL_HASWELL_EP     =  0x3F,
        CPU_MODEL_HASWELL_ULT    =  0x45,
        CPU_MODEL_BROADWELL      =  0x3D,
        CPU_MODEL_BROADWELL_ULX  =  0x3D,
        CPU_MODEL_BROADWELL_ULT  =  0x3D,
        CPU_MODEL_BRYSTALWELL    =  0x47,
        CPU_MODEL_SKYLAKE        =  0x4E,
        CPU_MODEL_SKYLAKE_ULT    =  0x4E,
        CPU_MODEL_SKYLAKE_ULX    =  0x4E,
        CPU_MODEL_SKYLAKE_DT     =  0x5E,
        CPU_MODEL_SKYLAKE_W      =  0x55,
        CPU_MODEL_KABYLAKE       =  0x8E,
        CPU_MODEL_KABYLAKE_ULT   =  0x8E,
        CPU_MODEL_KABYLAKE_ULX   =  0x8E,
        CPU_MODEL_KABYLAKE_DT    =  0x9E,
        CPU_MODEL_COFFEELAKE     =  0x9E,
        CPU_MODEL_COFFEELAKE_ULT =  0x9E,
        CPU_MODEL_COFFEELAKE_ULX =  0x9E,
        CPU_MODEL_COFFEELAKE_DT  =  0x9E,
        CPU_MODEL_CANNONLAKE     =  0x66,
        CPU_MODEL_ICELAKE        =  0x7E
    };
};

#endif /* CPUInfo_hpp */
