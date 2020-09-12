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
    var settingsEntity: SettingsEntity!
    
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
        
        let cpuCount = getPhysicalCPUCount()
        
        self.settingsEntity = SettingsEntity()
        do {
            try self.settingsEntity.load()
            mapSettingsEntityToDataStore(self.settingsEntity, self.viewDataStore, cpuCount)
        } catch {
            self.viewDataStore.showErrorForSettings = true
            self.viewDataStore.errorMsgForSettings = error.localizedDescription
        }
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
            do {
                try self.saveSettings()
                self.settingsWindow.orderOut(nil)
            } catch {
                self.viewDataStore.showErrorForSettings = true
                self.viewDataStore.errorMsgForSettings = error.localizedDescription
            }
        }
        self.viewDataStore.settingsApply = {
            do {
                try self.saveSettings()
            } catch {
                self.viewDataStore.showErrorForSettings = true
                self.viewDataStore.errorMsgForSettings = error.localizedDescription
            }
        }
    }
    
    private func saveSettings() throws {
        mapDataStoreToSettingsEntity(self.viewDataStore, self.settingsEntity)
        
        try self.settingsEntity.persist()
    }
}
