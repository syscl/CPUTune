//
//  SIPTune.hpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#ifndef SIPTune_hpp
#define SIPTune_hpp

#include <pexpert/pexpert.h>
#include "csr.h"

class SIPTune {
public:
    SIPTune();
    ~SIPTune();
    
    bool getCSRActiveConfig(uint32_t flag) const;
    void allowUnrestrictedFS(void) { setBootCSRActiveConfig(CSR_ALLOW_UNRESTRICTED_FS); };
    void allowUntrustedKexts(void) { setBootCSRActiveConfig(CSR_ALLOW_UNTRUSTED_KEXTS); };
    
    
private:
    uint32_t *gBootCSRActiveConfig = nullptr;
    uint32_t *getBootCSRActiveConfig(void) const;
    
    // Combination usage e.g. flag = (CSR_ALLOW_UNTRUSTED_KEXTS + CSR_ALLOW_UNRESTRICTED_FS)
    void setBootCSRActiveConfig(const uint32_t flag);
};

#endif /* SIPTune_hpp */
