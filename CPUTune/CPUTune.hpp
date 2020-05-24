//
//  CPUTune.hpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#ifndef CPUTune_hpp
#define CPUTune_hpp

#include <IOKit/IOService.h>
#include <CPUInfo.hpp>
#include <SIPTune.hpp>
#include <NVRAMUtils.hpp>

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
    const char *ProcHotPath = nullptr;
    const char *speedShiftPath = nullptr;
    bool enableIntelTurboBoost = true;
    bool enableIntelProcHot = false;
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
    
    static constexpr uint64_t kDisableProcHotBit = 0xFFFFFFFE;
    static constexpr uint64_t kEnableProcHotBit = 0x1;
    
    
    IOWorkLoop *myWorkLoop;
    IOTimerEventSource *timerSource;
    void readConfigAtRuntime(OSObject *owner, IOTimerEventSource *sender);
    
    
    void enableTurboBoost(void);
    void disableTurboBoost(void);
    
    void enableProcHot(void);
    void disableProcHot(void);
    
    void enableSpeedShift(void);
    void disableSpeedShift(void);
    
    bool setIfNotEqual(const uint64_t, const uint64_t, const uint32_t) const;
    
    // As per Apple, don't declare default constructor.
    // The default constuctor CPUTune() will do the following
    // explicitly: cpu_info = CPUInfo(); ...
    // This avoid construct/destruct the class twice
    CPUInfo cpu_info;
    SIPTune sip_tune;
    NVRAMUtils nvram;
    
    bool allowUnrestrictedFS = false;
    
    uint64_t org_MSR_IA32_MISC_ENABLE;
    uint64_t org_MSR_IA32_PERF_CTL;
    uint64_t org_MSR_IA32_POWER_CTL;
    
    uint64_t org_MSR_IA32_PM_ENABLE;
    
    template<typename T>
    static void deleter(T *&p) {
        delete p;
        p = nullptr;
    }
};

#endif /* CPUTune_hpp */
