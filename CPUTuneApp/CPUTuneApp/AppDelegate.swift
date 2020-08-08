//
//  AppDelegate.swift
//  CPUTuneApp
//
//  Created by Yating Zhou on 7/5/20.
//  Copyright © 2020 syscl. All rights reserved.
//

import Cocoa
import SwiftUI

let pathCPUTuneTurboBoostRT: String = "/tmp/CPUTuneTurboBoostRT.conf"
let pathHWPRequest: String = "/tmp/HWPRequest.conf"
let pathTurboRatioLimit: String = "/tmp/TurboRatioLimit.conf"
let pathCPUTuneProcHotRT: String = "/tmp/CPUTuneProcHotRT.conf"

func getPhysicalCPUCount() -> Int {
    var resultSize = 0
    sysctlbyname("hw.physicalcpu", nil, &resultSize, nil, 0)
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

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
    var viewDataStore: ViewDataStore!
    
    var window: NSWindow!
    var settingsWindow: NSWindow!
    var statusItem: NSStatusItem!

    @IBOutlet weak var stateBarMenu: NSMenu!
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        self.initViewDataStore()
        
        self.initStateBarMenu()
        self.initStteingsWindow()
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }
    
    @IBAction func stateBarSettingsOnClick(_ sender: NSMenuItem) {
        self.settingsWindow.makeKeyAndOrderFront(nil)
    }
    
    private func initViewDataStore() {
        self.viewDataStore = ViewDataStore()
        
        let settingValues = getSettings()

        self.viewDataStore.enableTurboBoost = settingValues.0
        self.viewDataStore.hwpRequestValue = settingValues.1
        self.viewDataStore.enableProcHot = settingValues.3
        let strTurboRatioLimits = settingValues.2
        
        let cpuCount = getPhysicalCPUCount()
        
        var trlList = Array(
            repeating: TRLItem(id: 0, value: ""),
            count: cpuCount
        )
        
        for i in 0..<cpuCount {
            var limitValue: String = ""
            if strTurboRatioLimits.count > i * 2 {
                limitValue = subString(strTurboRatioLimits, i * 2, i * 2 + 2)
            }
            trlList[i] = TRLItem(id: i, value: limitValue)
        }
        self.viewDataStore.turboRatioLimits = trlList
    }
    
    private func initStateBarMenu() {
        self.statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.squareLength)
        if let button = self.statusItem.button {
            button.image = NSImage(named: "StatusIcon")
        }
        self.statusItem.menu = self.stateBarMenu
    }
    
    private func initStteingsWindow() {
        self.settingsWindow = NSWindow(
            contentRect: NSRect(x: 0, y: 0, width: 500, height: 300),
            styleMask: [.titled, .closable, .miniaturizable, .resizable, .fullSizeContentView],
            backing: .buffered, defer: false)
        self.settingsWindow.center()
        self.settingsWindow.setFrameAutosaveName("Settings")
        
        self.settingsWindow.contentView = NSHostingView(
            rootView: SettingsView().environmentObject(self.viewDataStore)
        )
        
        self.settingsWindow.title = NSLocalizedString("Settings", comment: "Settings")
        self.settingsWindow.standardWindowButton(NSWindow.ButtonType.closeButton)?.isHidden = true
        self.settingsWindow.standardWindowButton(NSWindow.ButtonType.miniaturizeButton)?.isHidden = true
        self.settingsWindow.standardWindowButton(NSWindow.ButtonType.zoomButton)?.isHidden = true
        self.settingsWindow.level = .floating
        
        self.viewDataStore.settingsCancel = {
            self.settingsWindow.orderOut(nil)
        }
        self.viewDataStore.settingsConfirm = {
            self.settingsWindow.orderOut(nil)
            self.saveSettings()
        }
        self.viewDataStore.settingsApply = {
            self.saveSettings()
        }
    }
    
    private func saveSettings() {
        let strTurboRatioLimits: String = self.viewDataStore.turboRatioLimits.map {v in
            v.value
        }.joined()
        
        persistSettings(
            self.viewDataStore.enableTurboBoost,
            self.viewDataStore.hwpRequestValue,
            strTurboRatioLimits,
            self.viewDataStore.enableProcHot
        )
    }
}

final class ViewDataStore: ObservableObject {
    @Published var running: Bool = false
    
    @Published var enableTurboBoost: Bool = false
    @Published var enableProcHot: Bool = false
    @Published var updateTimeInterval: UInt32 = 0
    @Published var hwpRequestValue: String = ""
    @Published var turboRatioLimits: [TRLItem] = []
    @Published var enableRurboBoost: Bool = false
    
    var settingsCancel: () -> () = {}
    var settingsConfirm: () -> () = {}
    var settingsApply: () -> () = {}
}

struct TRLItem: Identifiable {
    let id: Int
    var value: String
}
