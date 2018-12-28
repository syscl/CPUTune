CPUTune Changelog
=======================
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
