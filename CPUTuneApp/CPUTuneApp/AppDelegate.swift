//
//  AppDelegate.swift
//  CPUTuneApp
//
//  Created by Yating Zhou on 7/5/20.
//  Copyright © 2020 syscl. All rights reserved.
//

import Cocoa
import SwiftUI

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
                limitValue = String(UInt(limitValue, radix: 16) ?? 0)
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
            String(format: "%0.2x", UInt(v.value) ?? 0)
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
    @Published var hwpRequestValue: String = ""
    @Published var turboRatioLimits: [TRLItem] = []
    
    var settingsCancel: () -> () = {}
    var settingsConfirm: () -> () = {}
    var settingsApply: () -> () = {}
}

struct TRLItem: Identifiable {
    let id: Int
    var value: String
}
