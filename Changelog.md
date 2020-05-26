CPUTune Changelog
=======================
#### v2.0.6

- Tune turbo ratio limit at runtime when turbo is enabled

#### v2.0.5

- Make code cleaner and shorter, remove unnecessary ```initKextPreferences()```

#### v2.0.4

- Make update timeout configurable so that we have a more looser/tigher control over the HWP request
- Move the ```initKextPreferences()``` up to ```CPUTune::init()```

#### v2.0.3

- Decouple HWP information to ```CPUInfo```

#### v2.0.2

- Support no ```0x/0X``` prefix hexadecimal constant (e.g. 0x800d3008 as 800d3008)

#### v2.0.1

- Verify and surface invalid HWP request properly

#### v2.0.0

- Major release, fix logging in current value after ```setIfNotEqual()```

#### v1.9.9

- Introduce HWP request at runtime

#### v1.9.8

- Avoid construct objects twice

#### v1.9.7

- Remove useless const qualifier + improved code style

#### v1.9.6

- Added init/restore for ```BD_PROCHOT```
- Unified the log output for PROCHOT and use ```setIfNotEqual()```
- Fixed a bug in ```readConfigAtRuntime``` which was not restoring ProcHot correctly when coming out of sleep mode

#### v1.9.5

- Fixed the return type of the ```setIfNotEqual()```

#### v1.9.4

- Simplify writing logic to MSR

#### v1.9.3
- Avoid dynamic allocation for CPUInfo and SIPTune, which avoid indefine blocking in kernel space + avoid memory leak
- Use type anotation

#### v1.9.2

- More accurate logging for non-exist propreties in NVRAM

#### v1.9.1

- Supports ```BD_PROCHOT``` signal on Intel CPU credits @christophe-duc
- Update README.md

#### v1.9.0

- Resolve linkage issue for kernel major/minor

#### v1.8.9

- NVRAM protection

#### v1.8.8

- Supported macOS Catalina

#### v1.8.7

- Updated cpu info to ```xnu-4903.221.2```

#### v1.8.6

- Updated csr header to ```xnu-4903.221.2```

#### v.1.8.5

- Simplify the enable if-condition

#### v1.8.4

- Removed unused variables
- Corrected  ```writeBufferToFile()``` return type

#### v1.8.1
- Regularly checks profiles

#### v1.8.0
- Allowed SIP control a bit easier via Info.plist tunning instead of reboot to recovery mode on a real mac
- Simplify memory deallocate code blocks via deleter routine

#### v1.7.0
- Fixed compatible issues (supported OS X Mountain Lion and Xcode 3.2)
- Fixed code logic of HWP Enable, details can be fond [here](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3b-part-2-manual.pdf)

#### v1.6.5
- Implemented Timer Event based feature for dynamic control CPU Performance at runtime 
- CPUInfo to detect cpu information

#### v1.1.1
- Implemented write logs to file

#### v1.0.7
- Boot arguments check 
- Unsupported version check 
- Fixed memeory leak

#### v 1.0.1
- Fixed enableTurboBoost().

#### v1.0.0
- Initial release
