
import Foundation

let pathCPUTuneTurboBoostRT: String = "/tmp/CPUTuneTurboBoostRT.conf"
let pathHWPRequest: String = "/tmp/HWPRequest.conf"
let pathTurboRatioLimit: String = "/tmp/TurboRatioLimit.conf"
let pathCPUTuneProcHotRT: String = "/tmp/CPUTuneProcHotRT.conf"

func getPhysicalCPUCount() -> Int {
    var resultSize = 0
    sysctlbyname("hw.physicalcpu", nil, &resultSize, nil, 0)
    if resultSize <= 0 {
        return 1
    }
    var resultCString = [CChar](repeating: 0, count: resultSize)
    sysctlbyname("hw.physicalcpu", &resultCString, &resultSize, nil, 0)
    return Int(resultCString[0])
}

func subString(_ inputString: String, _ start: Int, _ end: Int) -> String {
    let s = start < 0 ? 0 : (start > inputString.count ? inputString.count : start)
    let e = end > inputString.count ? inputString.count : (end < 0 ? 0 : end)
    return String(inputString[
        inputString.index(inputString.startIndex, offsetBy: s) ..<
        inputString.index(inputString.startIndex, offsetBy: e)
    ])
}

func subString(_ inputString: String, _ start: Int) -> String {
    return subString(inputString, start, inputString.count)
}

func getSettings() -> (Bool, String, String, Bool) {
    var turboBoost: String = "0"
    var hwpRequest: String = ""
    var turboRatioLimit: String = ""
    var procHot: String = "0"
    
    do {
        turboBoost = try String(contentsOfFile: pathCPUTuneTurboBoostRT, encoding: .ascii)
    } catch {
        turboBoost = "0"
    }
    
    do {
        hwpRequest = try String(contentsOfFile: pathHWPRequest, encoding: .ascii)
    } catch {
        hwpRequest = ""
    }

    do {
        turboRatioLimit = try String(contentsOfFile: pathTurboRatioLimit, encoding: .ascii)
    } catch {
        turboRatioLimit = ""
    }
    
    do {
        procHot = try String(contentsOfFile: pathCPUTuneProcHotRT, encoding: .ascii)
    } catch {
        procHot = "0"
    }

    return (
        turboBoost == "1",
        subString(hwpRequest, 2),
        subString(turboRatioLimit, 2),
        procHot == "1"
    )
}

func persistSettings(
    _ turboBoost: Bool,
    _ hwpRequest: String,
    _ turboRatioLimit: String,
    _ procHot: Bool
) {
    do {
        let stringContent = turboBoost ? "1" : "0"
        try stringContent.write(
            toFile: pathCPUTuneTurboBoostRT,
            atomically: false,
            encoding: .ascii
        )
    } catch {}

    do {
        try "0x\(hwpRequest)".write(
            toFile: pathHWPRequest,
            atomically: false,
            encoding: .ascii
        )
    } catch {}

    do {

        try "0x\(turboRatioLimit)".write(
            toFile: pathTurboRatioLimit,
            atomically: false,
            encoding: .ascii
        )
    } catch {}

    do {
        let stringContent = procHot ? "1" : "0"
        try stringContent.write(
            toFile: pathCPUTuneProcHotRT,
            atomically: false,
            encoding: .ascii
        )
    } catch {}
}

