CPUTune Changelog
=======================
### v1.9.5

- Fixed the return type of the ```setIfNotEqual()```

### v1.9.4

- Simplify writing logic to MSR

### v1.9.3
- Avoid dynamic allocation for CPUInfo and SIPTune, which avoid indefine blocking in kernel space + avoid memory leak
- Use type anotation

### v1.9.2

- More accurate logging for non-exist propreties in NVRAM

### v1.9.1

- Supports ```BD_PROCHOT``` signal on Intel CPU credits @christophe-duc
- Update README.md

### v1.9.0

- Resolve linkage issue for kernel major/minor

### v1.8.9

- NVRAM protection

### v1.8.8

- Supported macOS Catalina

### v1.8.7

- Updated cpu info to ```xnu-4903.221.2```

### v1.8.6

- Updated csr header to ```xnu-4903.221.2```

### v.1.8.5

- Simplify the enable if-condition

### v1.8.4

- Removed unused variables
- Corrected  ```writeBufferToFile()``` return type

### v1.8.1
- Regularly checks profiles

### v1.8.0
- Allowed SIP control a bit easier via Info.plist tunning instead of reboot to recovery mode on a real mac
- Simplify memory deallocate code blocks via deleter routine

### v1.7.0
- Fixed compatible issues (supported OS X Mountain Lion and Xcode 3.2)
- Fixed code logic of HWP Enable, details can be fond [here](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3b-part-2-manual.pdf)

### v1.6.5
- Implemented Timer Event based feature for dynamic control CPU Performance at runtime 
- CPUInfo to detect cpu information

### v1.1.1
- Implemented write logs to file

### v1.0.7
- Boot arguments check 
- Unsupported version check 
- Fixed memeory leak

### v 1.0.1
- Fixed enableTurboBoost().

#### v1.0.0
- Initial release
