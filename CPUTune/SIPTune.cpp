//
//  SIPTune.cpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#include "SIPTune.hpp"

SIPTune::SIPTune() : gBootCSRActiveConfig(getBootCSRActiveConfig())
{
    
}

SIPTune::~SIPTune()
{
    // we cannot delete it otherwise we remove CSRActiveConfig in boot_args
    gBootCSRActiveConfig = nullptr;
}

uint32_t *SIPTune::getBootCSRActiveConfig(void) const
{
    auto bootArgs = static_cast<boot_args *>(PE_state.bootArgs);
    if (bootArgs) {
        return &(bootArgs->csrActiveConfig);
    }
    return nullptr;
}

bool SIPTune::getCSRActiveConfig(uint32_t flag) const
{
    return gBootCSRActiveConfig && ((*gBootCSRActiveConfig) & flag);
}

void SIPTune::setBootCSRActiveConfig(const uint32_t flag)
{
    uint32_t *const csr = gBootCSRActiveConfig;
    if (csr) {
        (*csr) |= flag;
    }
}


