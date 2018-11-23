//
//  kern_util.hpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#ifndef kern_util_hpp
#define kern_util_hpp

#include <IOKit/IOLib.h>

#define xStringify(a) Stringify(a)
#define Stringify(a) #a

#define xConcat(a, b) Concat(a, b)
#define Concat(a, b) a ## b

/**
 *  Prefix name with your plugin name (to ease symbolication and avoid conflicts)
 */
#define ADDPR(a) xConcat(xConcat(PRODUCT_NAME, _), a)

/**
 *  Debugging state exported for your plugin
 */
extern bool ADDPR(debugEnabled);

/**
 *  Debugging print delay used as an ugly hack around printf bufferisation,
 *  which results in messages not appearing in the boot log.
 */
extern uint32_t ADDPR(debugPrintDelay);

/**
 *  Write to system log with kernel extension name
 *
 *  @param str    printf-like string
 */
#define myLOG(str, ...)                                                                          \
    do {                                                                                         \
        cputune_os_log("CPUTune @ " str "\n", ## __VA_ARGS__);                                   \
    } while (0)


/**
 *  Write to system log
 *
 *  @param str    printf-like string
 */
#define SYSLOG(str, ...)                                                                         \
    do {                                                                                         \
        cputune_os_log(str "\n", ## __VA_ARGS__);                                                \
    } while (0)

/**
 *  Export function or symbol for linking
 */
#define EXPORT __attribute__((visibility("default")))

/**
 *  Remove padding between fields
 */
#define PACKED __attribute__((packed))

/**
 *  This function is supposed to workaround missing entries in the system log.
 *  By providing its own buffer for logging data.
 *
 *  @param format  formatted string
 */
EXPORT extern "C" void cputune_os_log(const char *format, ...);

#endif /* kern_util_hpp */
