//
//  kern_util.cpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#include "kern_util.hpp"
#include <sys/types.h>

bool ADDPR(debugEnabled) = false;
uint32_t ADDPR(debugPrintDelay) = 0;

void cputune_os_log(const char *format, ...) {
    char tmp[1024];
    tmp[0] = '\0';
    va_list va;
    va_start(va, format);
    vsnprintf(tmp, sizeof(tmp), format, va);
    va_end(va);
    
    if (ml_get_interrupts_enabled())
        IOLog("%s", tmp);
    
    if (ml_get_interrupts_enabled() && ADDPR(debugPrintDelay) > 0)
        IOSleep(ADDPR(debugPrintDelay));
}
