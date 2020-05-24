//
//  kern_util.hpp
//  CPUTune
//
//  Copyright (c) 2018 syscl. All rights reserved.
//

#ifndef kern_util_hpp
#define kern_util_hpp

#include <IOKit/IOLib.h>
#include <libkern/version.h>

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

/**
 *  Possible boot arguments
 */
static constexpr const char *bootargOff  {"-cputoff"};          // Disable the kext
static constexpr const char *bootargBeta {"-cputbeta"};         // Force enable the kext on unsupported os

/**
 *  Known kernel versions
 */
enum KernelVersion {
    SnowLeopard   = 10,
    Lion          = 11,
    MountainLion  = 12,
    Mavericks     = 13,
    Yosemite      = 14,
    ElCapitan     = 15,
    Sierra        = 16,
    HighSierra    = 17,
    Mojave        = 18,
    Catalina      = 19,
    Unsupported
};

/**
 *  Kernel minor version for symmetry
 */
using KernelMinorVersion = int;

/**
 *  Obtain major kernel version
 *
 *  @return numeric kernel version
 */
inline KernelVersion getKernelVersion() {
    return static_cast<KernelVersion>(version_major);
}

/**
 *  Obtain minor kernel version
 *
 *  @return numeric minor kernel version
 */
inline KernelMinorVersion getKernelMinorVersion() {
    return static_cast<KernelMinorVersion>(version_minor);
}

/**
 *  Check whether kernel boot argument is passed ignoring the value (e.g. -arg or arg).
 *
 *  @param name  argument name
 *
 *  @return true if argument was passed
 */
inline bool checkKernelArgument(const char *name) {
    int val[16];
    return PE_parse_boot_argn(name, val, sizeof(val));
}

/**
 *  Parse apple version at compile time
 *
 *  @param version string literal representing apple version (e.g. 1.1.1)
 *
 *  @return numeric kernel version
 */
constexpr size_t parseModuleVersion(const char *version) {
    return (version[0] - '0') * 100 + (version[2] - '0') * 10 + (version[4] - '0');
}

/**
*  Convert a hexicimal string to a long integer
*
*  @param hex string literal representing the hex value
*
*  @return long integer base 10
*/
inline long hexToInt(const char *hex) {
    int base = 16;
    for (const char *c = hex; *c != '\0'; c++) {
        if (*c == 'x' || *c== 'X') {
            base = 0;
            break;
        }
    }
    return strtol(hex, NULL, base);
}

/**
 *  Slightly non-standard helpers to get the date in a YYYY-MM-DD format.
 */
template <size_t i>
inline constexpr char getBuildYear() {
    static_assert(i < 4, "Year consists of four digits");
    return __DATE__[7+i];
}

template <size_t i>
inline constexpr char getBuildMonth() {
    static_assert(i < 2, "Month consists of two digits");
    auto mon = *reinterpret_cast<const uint32_t *>(__DATE__);
    switch (mon) {
        case ' naJ':
            return "01"[i];
        case ' beF':
            return "02"[i];
        case ' raM':
            return "03"[i];
        case ' rpA':
            return "04"[i];
        case ' yaM':
            return "05"[i];
        case ' nuJ':
            return "06"[i];
        case ' luJ':
            return "07"[i];
        case ' guA':
            return "08"[i];
        case ' peS':
            return "09"[i];
        case ' tcO':
            return "10"[i];
        case ' voN':
            return "11"[i];
        case ' ceD':
            return "12"[i];
    }
    
    return '0';
}

template <size_t i>
inline constexpr char getBuildDay() {
    static_assert(i < 2, "Day consists of two digits");
    if (i == 0 && __DATE__[4+i] == ' ')
        return '0';
    return __DATE__[4+i];
}

static const char kextVersion[] {
#ifdef DEBUG
    'D', 'B', 'G', '-',
#else
    'R', 'E', 'L', '-',
#endif
    xStringify(MODULE_VERSION)[0], xStringify(MODULE_VERSION)[2], xStringify(MODULE_VERSION)[4], '-',
    getBuildYear<0>(), getBuildYear<1>(), getBuildYear<2>(), getBuildYear<3>(), '-',
    getBuildMonth<0>(), getBuildMonth<1>(), '-', getBuildDay<0>(), getBuildDay<1>(), '\0'
};

/**
 *  Reads file data at path with given length
 *
 *  @param path full file path
 *  @param off offset of a file
 *  @param bytes bytes read
 *
 *  @return allocated buffer on success or nullptr on error
 */
EXPORT uint8_t *readFileNBytes(const char* path, off_t off, size_t bytes);


#endif /* kern_util_hpp */
