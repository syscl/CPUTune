//
//  CPUTune.hpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#ifndef CPUTune_hpp
#define CPUTune_hpp

#include <IOKit/IOService.h>

class CPUTune : public IOService
{
    OSDeclareDefaultStructors(CPUTune)
    using super = IOService;
    
public:
    virtual bool init(OSDictionary *dict) override;
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    virtual void free(void) override;
    
private:
    uint64_t getTurboBoostState(void) const;
    void enableTurboBoost(void);
    void disableTurboBoost(void);
    
protected:
    uint64_t org_MSR_IA32_MISC_ENABLE;
    uint64_t org_MSR_IA32_PERF_CTL;
};

#endif /* CPUTune_hpp */
