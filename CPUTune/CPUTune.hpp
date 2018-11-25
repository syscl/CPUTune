//
//  CPUTune.hpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#ifndef CPUTune_hpp
#define CPUTune_hpp

#include <IOKit/IOService.h>
#include "CPUInfo.hpp"

class CPUTune : public IOService
{
    OSDeclareDefaultStructors(CPUTune)
    using super = IOService;
    
public:
    IOService *probe(IOService *provider, SInt32 *score) override;
    virtual bool init(OSDictionary *dict) override;
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    virtual void free(void) override;
    
private:
    const char *turboBoostPath = nullptr;
    const char *speedShiftPath = nullptr;
    bool enableIntelTurboBoost = true;
    bool supportedSpeedShift = false;
    // As 64-ia-32-architectures-software-developer-vol-3b-part-2-manual (Vol. 3B 14-7)
    // Only RESET will clear this bit.
    bool enableIntelSpeedShift = true;
    bool hwpEnableOnceSet = false;
    
    void initKextPerferences();
    
    static constexpr uint64_t kEnableTurboBoostBits  = ((uint64_t)-1) ^ ((uint64_t)1) << 38;
    static constexpr uint64_t kDisableTurboBoostBits = ~kEnableTurboBoostBits;
    
    static constexpr uint64_t kEnableSpeedShiftBit  = 0x1;
    static constexpr uint64_t kDisableSpeedShiftBit = 0;
    
    
    IOWorkLoop *myWorkLoop;
    IOTimerEventSource *timerSource;
    void readConfigAtRuntime(OSObject *owner, IOTimerEventSource *sender);
    bool logTurboBoostConfMissing = false;
    bool logSpeedShiftConfMissing = false;
    
    
    void enableTurboBoost(void);
    void disableTurboBoost(void);
    
    void enableSpeedShift(void);
    void disableSpeedShift(void);
    
    CPUInfo *cpu_info {nullptr};
    
    uint64_t org_MSR_IA32_MISC_ENABLE;
    uint64_t org_MSR_IA32_PERF_CTL;
    
    uint64_t org_MSR_IA32_PM_ENABLE;
};

#endif /* CPUTune_hpp */
