
CPUTune
====
An open source kernel extension enables dynamic CPU performance tuning at runtime for macOS.

#### Features
- Allows tuning on or off Intel Turbo Boost for better battery life
- Allows tuning on or off Intel Speed Shift for maximum performance
- Implements TimerEvent-based responses for dynamical switching Turbo Boost and Speed Shift at runtime
- Allows System Integrity Protection (SIP) control a bit easier via Info.plist setting 

#### Boot arguments
- Add `-cputoff` to disable CPUTune
- Add `-cputbeta` to enable CPUTune on unsupported os versions (10.14 and below are enabled by default).

### Configuration
- In terminal, type in ```echo "1">/tmp/CPUTuneTurboBoostRT.conf``` to enable turbo boost when CPUTune is loaded
- In terminal, type in ```echo "0">/tmp/CPUTuneTurboBoostRT.conf``` to disable turbo boost when needed
- In case you want a simplify command to switch turbo boost, change the `TurboBoostAtRuntime` in `CPUTune.kext/Contents/Info.plist`

#### Contribution
All suggestions and improvements are welcome, don't hesitate to pull request or open an issue if you want this project to be better than ever.
Writing and supporting code is fun but it takes time. Please provide most descriptive bugreports or pull requests.


#### Change Log
[Change logs](https://github.com/syscl/CPUTune/blob/master/Changelog.md) for detail improvements

