//
//  SettingsEntity.swift
//  CPUTuneApp
//

import Foundation

let pathCPUTuneTurboBoostRT: String = "/tmp/CPUTuneTurboBoostRT.conf"
let pathHWPRequest: String = "/tmp/HWPRequest.conf"
let pathTurboRatioLimit: String = "/tmp/TurboRatioLimit.conf"
let pathCPUTuneProcHotRT: String = "/tmp/CPUTuneProcHotRT.conf"

class SettingsEntity {
    private var _turboBoost: Bool = false
    private var _hwpRequest: String = ""
    private var _turboRatioLimit: String = ""
    private var _procHot: Bool = false
    
    var turboBoost: Bool {
        get {
            return self._turboBoost;
        }
        set {
            self._turboBoost = newValue;
        }
    }
    
    var hwpRequest: String {
        get {
            return self._hwpRequest;
        }
        set {
            if newValue.count > 8 {
                self._hwpRequest = subString(newValue, 0, 8);
            } else {
                self._hwpRequest = newValue;
            }
        }
    }
    
    var procHot: Bool {
        get {
            return self._procHot;
        }
        set {
            self._procHot = newValue;
        }
    }
    
    func getTurboRatioLimitAsArray() -> Array<UInt> {
        if self._turboRatioLimit.count < 1 {
            return []
        }
        let n = self._turboRatioLimit.count / 2
        var values = Array<UInt>(repeating: 0, count: n)
        for i in 0..<n {
            let limitValue = subString(self._turboRatioLimit, i * 2, i * 2 + 2)
            values[i] = UInt(limitValue, radix: 16) ?? 0
        }
        return values
    }
    
    func setTurboRatioLimitByArray(_ values: Array<UInt>) {
        self._turboRatioLimit = values.map {v in
            String(format: "%0.2x", min(v, 255))
        }.joined()
    }
    
    func load() throws {
        var turboBoost: String = "0"
        var hwpRequest: String = ""
        var turboRatioLimit: String = ""
        var procHot: String = "0"
        
        var errorMessages: Array<String> = []
        
        do {
            turboBoost = try String(contentsOfFile: pathCPUTuneTurboBoostRT, encoding: .ascii)
        } catch {
            errorMessages.append(error.localizedDescription)
        }
        
        do {
            hwpRequest = try String(contentsOfFile: pathHWPRequest, encoding: .ascii)
        } catch {
            errorMessages.append(error.localizedDescription)
        }

        do {
            turboRatioLimit = try String(contentsOfFile: pathTurboRatioLimit, encoding: .ascii)
        } catch {
            errorMessages.append(error.localizedDescription)
        }
        
        do {
            procHot = try String(contentsOfFile: pathCPUTuneProcHotRT, encoding: .ascii)
        } catch {
            errorMessages.append(error.localizedDescription)
        }
        
        if errorMessages.count > 0 {
            throw ErrorWithMessage(errorMessages.joined(separator: "\n"))
        }

        self._turboBoost = turboBoost == "1"
        self._hwpRequest = subString(hwpRequest, 2)
        self._turboRatioLimit = subString(turboRatioLimit, 2)
        self._procHot = procHot == "1"
    }
    
    func persist() throws {
        var errorMessages: Array<String> = []
        
        do {
            let stringContent = self._turboBoost ? "1" : "0"
            try stringContent.write(
                toFile: pathCPUTuneTurboBoostRT,
                atomically: false,
                encoding: .ascii
            )
        } catch {
            errorMessages.append(error.localizedDescription)
        }

        do {
            try "0x\(self._hwpRequest)".write(
                toFile: pathHWPRequest,
                atomically: false,
                encoding: .ascii
            )
        } catch {
            errorMessages.append(error.localizedDescription)
        }

        do {

            try "0x\(self._turboRatioLimit)".write(
                toFile: pathTurboRatioLimit,
                atomically: false,
                encoding: .ascii
            )
        } catch {
            errorMessages.append(error.localizedDescription)
        }

        do {
            let stringContent = self._procHot ? "1" : "0"
            try stringContent.write(
                toFile: pathCPUTuneProcHotRT,
                atomically: false,
                encoding: .ascii
            )
        } catch {
            errorMessages.append(error.localizedDescription)
        }
        
        if errorMessages.count > 0 {
            throw ErrorWithMessage(errorMessages.joined(separator: "\n"))
        }
    }
}
