//
//  NVRAMUtils.hpp
//  CPUTune
//
//  Created by Yating Zhou on 3/22/20.
//  Copyright © 2020 syscl. All rights reserved.
//

#ifndef NVRAMUtils_hpp
#define NVRAMUtils_hpp

#include <stddef.h>
#include <IOKit/IONVRAM.h>

static constexpr char kCPUTUNE_PANIC_KEY[] = "cpu-tune-panic";

class NVRAMUtils {
public:
    int getProperty(const char *symbol, void *value, size_t *len) const;
    int setProperty(const char *symbol, const void * value, size_t len) const;
    int removeProperty(const char *symbol) const;
    bool setKextPanicKey(void) const;
    bool clearKextPanicKey(void) const;
    bool isKextPanicLastBoot(void) const;
    
private:
    IODTNVRAM *getNVRAMEntry(void) const;
};


#endif /* NVRAMUtils_hpp */
