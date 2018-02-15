//
//  LowResNXWindowController.swift
//  LowRes NX macOS
//
//  Created by Timo Kloss on 30/4/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import Cocoa

class LowResNXWindowController: NSWindowController, NSWindowDelegate, CoreWrapperDelegate {
    @IBOutlet weak var lowResNXView: LowResNXView!
    @IBOutlet weak var backgroundView: NSView!
    @IBOutlet weak var widthConstraint: NSLayoutConstraint!
    
    var timer: Timer?
    var coreWrapper: CoreWrapper?
    var nxDiskUrl: URL?
    var nxDiskDate: Date?

    override func windowDidLoad() {
        super.windowDidLoad()
        window!.delegate = self
        
        backgroundView.wantsLayer = true
        backgroundView.layer?.backgroundColor = CGColor(gray: 0, alpha: 1)
        
        let lowResNXDocument = document as! LowResNXDocument
        coreWrapper = lowResNXDocument.coreWrapper
        
        if let coreWrapper = coreWrapper {
            coreWrapper.delegate = self
            
            let secondsSincePowerOn = -(NSApp.delegate as! AppDelegate).launchDate.timeIntervalSinceNow
            core_willRunProgram(&coreWrapper.core, Int(secondsSincePowerOn))
            
            timer = Timer.scheduledTimer(timeInterval: 1.0/60.0, target: self, selector: #selector(LowResNXWindowController.update), userInfo: nil, repeats: true)
        }
    }
    
    func windowWillClose(_ notification: Notification) {
        timer?.invalidate()
    }
    
    func windowDidResize(_ notification: Notification) {
        if let contentView = window?.contentView {
            let screenWidth = contentView.bounds.size.width
            let screenHeight = contentView.bounds.size.height
            let maxWidthFactor = floor(screenWidth / CGFloat(SCREEN_WIDTH))
            let maxHeightFactor = floor(screenHeight / CGFloat(SCREEN_HEIGHT))
            widthConstraint.constant = (maxWidthFactor < maxHeightFactor) ? maxWidthFactor * CGFloat(SCREEN_WIDTH) : maxHeightFactor * CGFloat(SCREEN_WIDTH)
        }
    }
    
    @objc func update() {
        if let coreWrapper = coreWrapper {
            core_update(&coreWrapper.core, &coreWrapper.input)
            lowResNXView.render(coreWrapper: coreWrapper)
        }
    }
    
    override func keyDown(with event: NSEvent) {
        guard let coreWrapper = coreWrapper else {
            return
        }
        
        switch event.keyCode {
        case 123:
            coreWrapper.input.gamepads.0.left = true
        case 124:
            coreWrapper.input.gamepads.0.right = true
        case 125:
            coreWrapper.input.gamepads.0.down = true
        case 126:
            coreWrapper.input.gamepads.0.up = true
        case 6, 43: // Z, ,
            coreWrapper.input.gamepads.0.buttonA = true
        case 7, 47: // X, .
            coreWrapper.input.gamepads.0.buttonB = true
        case 2: // D
            coreWrapper.input.gamepads.1.left = true
        case 5: // G
            coreWrapper.input.gamepads.1.right = true
        case 3: // F
            coreWrapper.input.gamepads.1.down = true
        case 15: // R
            coreWrapper.input.gamepads.1.up = true
        case 0: // A
            coreWrapper.input.gamepads.1.buttonA = true
        case 1: // S
            coreWrapper.input.gamepads.1.buttonB = true
        default:
            break
        }
        let characters = event.charactersIgnoringModifiers!
        if !characters.isEmpty {
            if characters == "\r" {
                coreWrapper.input.key = CoreInputKeyReturn
            } else if characters == "\u{7F}" {
                coreWrapper.input.key = CoreInputKeyBackspace
            } else {
                let text = characters.uppercased()
                let codes = text.unicodeScalars
                let key = codes[codes.startIndex]
                if key.value < 127 {
                    coreWrapper.input.key = Int8(key.value)
                }
            }
            if characters == "p" || characters == "P" {
                coreWrapper.input.pause = true
            }
        }
    }
    
    override func keyUp(with event: NSEvent) {
        guard let coreWrapper = coreWrapper else {
            return
        }
        
        switch event.keyCode {
        case 123:
            coreWrapper.input.gamepads.0.left = false
        case 124:
            coreWrapper.input.gamepads.0.right = false
        case 125:
            coreWrapper.input.gamepads.0.down = false
        case 126:
            coreWrapper.input.gamepads.0.up = false
        case 6, 43: // Z, ,
            coreWrapper.input.gamepads.0.buttonA = false
        case 7, 47: // X, .
            coreWrapper.input.gamepads.0.buttonB = false
        case 2: // D
            coreWrapper.input.gamepads.1.left = false
        case 5: // G
            coreWrapper.input.gamepads.1.right = false
        case 3: // F
            coreWrapper.input.gamepads.1.down = false
        case 15: // R
            coreWrapper.input.gamepads.1.up = false
        case 0: // A
            coreWrapper.input.gamepads.1.buttonA = false
        case 1: // S
            coreWrapper.input.gamepads.1.buttonB = false
        default:
            break
        }
    }
    
    override func mouseDragged(with event: NSEvent) {
        if let coreWrapper = coreWrapper {
            let point = screenPoint(event: event)
            coreWrapper.input.touchX = Int32(point.x)
            coreWrapper.input.touchY = Int32(point.y)
        }
    }
    
    override func mouseDown(with event: NSEvent) {
        if let coreWrapper = coreWrapper {
            let point = screenPoint(event: event)
            if point.y >= 0 {
                coreWrapper.input.touch = true
                coreWrapper.input.touchX = Int32(point.x)
                coreWrapper.input.touchY = Int32(point.y)
            }
        }
    }
    
    override func mouseUp(with event: NSEvent) {
        if let coreWrapper = coreWrapper {
            coreWrapper.input.touch = false
        }
    }
    
    func screenPoint(event: NSEvent) -> CGPoint {
        let point = event.locationInWindow
        let viewPoint = window!.contentView!.convert(point, to: lowResNXView)
        let x = viewPoint.x * CGFloat(SCREEN_WIDTH) / lowResNXView.bounds.size.width;
        let y = CGFloat(SCREEN_HEIGHT) - viewPoint.y * CGFloat(SCREEN_HEIGHT) / lowResNXView.bounds.size.height;
        return CGPoint(x: x, y: y);
    }
    
    func loadDisk(diskDataManager: UnsafeMutablePointer<DataManager>?) {
        guard let nxDiskUrl = self.nxDiskUrl else {
            return
        }
        
        nxDiskDate = fileModificationDate(url: nxDiskUrl) ?? Date()
        
        do {
            let diskString = try String(contentsOf: nxDiskUrl)
            let chars = diskString.cString(using: .ascii)
            let error = data_import(diskDataManager, chars, true)
            if error.code != ErrorNone {
                let nxDocument = document as! LowResNXDocument
                throw LowResNXError(error: error, sourceCode: nxDocument.sourceCode)
            }
        } catch let error as NSError {
            presentError(error)
        }
    }
    
    func fileModificationDate(url: URL) -> Date? {
        let attrs = try? FileManager.default.attributesOfItem(atPath: url.path) as NSDictionary
        return attrs?.fileModificationDate()
    }
    
    @IBAction func toggleDebug(_ sender: Any) {
        if let coreWrapper = coreWrapper {
            core_setDebug(&coreWrapper.core, !core_getDebug(&coreWrapper.core))
        }
    }
    
    // MARK: - Core Wrapper Delegate
    
    func coreInterpreterDidFail(coreError: CoreError) -> Void {
        if let nxDocument = document as? LowResNXDocument {
            presentError(LowResNXError(error: coreError, sourceCode: nxDocument.sourceCode))
        }
    }
    
    func coreDiskDriveWillAccess(diskDataManager: UnsafeMutablePointer<DataManager>?) -> Bool {
        if let nxDiskUrl = nxDiskUrl {
            if let modificationDate = fileModificationDate(url: nxDiskUrl), modificationDate > nxDiskDate! {
                loadDisk(diskDataManager: diskDataManager)
            }
            return true
        }
        
        let panel = NSOpenPanel()
        panel.message = "Select an NX file whose data you want to edit."
        panel.prompt = "Use as Disk"
        panel.allowedFileTypes = ["nx"]
        panel.beginSheetModal(for: window!, completionHandler: { (response) in
            if response == .OK {
                self.nxDiskUrl = panel.url
            } else {
                let nxDocument = self.document as! LowResNXDocument
                let nxDiskUrl = nxDocument.fileURL!.deletingLastPathComponent().appendingPathComponent("Disk.nx")
                if !FileManager.default.fileExists(atPath: nxDiskUrl.path) {
                    FileManager.default.createFile(atPath: nxDiskUrl.path, contents: nil, attributes: nil)
                }
                self.nxDiskUrl = nxDiskUrl
            }
            
            self.loadDisk(diskDataManager: diskDataManager)
            
            core_diskLoaded(&self.coreWrapper!.core)
        })
        return false
    }
    
    func coreDiskDriveDidSave(diskDataManager: UnsafeMutablePointer<DataManager>?) -> Void {
        if let nxDiskUrl = nxDiskUrl {
            let output = data_export(diskDataManager)
            if let output = output {
                let data = Data(bytes: output, count: Int(strlen(output)))
                do {
                    try data.write(to: nxDiskUrl)
                    nxDiskDate = fileModificationDate(url: nxDiskUrl) ?? Date()
                } catch let error as NSError {
                    presentError(error)
                }
                free(output);
            } else {
                //TODO
            }
        } else {
            //TODO
        }
    }
    
    func coreControlsDidChange(controlsInfo: ControlsInfo) -> Void {
        
    }
    
}


