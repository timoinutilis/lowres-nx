//
//  LowResNXWindowController.swift
//  LowRes NX macOS
//
//  Created by Timo Kloss on 30/4/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import Cocoa

class LowResNXWindowController: NSWindowController, NSWindowDelegate {
    @IBOutlet weak var lowResNXView: LowResNXView!
    @IBOutlet weak var backgroundView: NSView!
    @IBOutlet weak var widthConstraint: NSLayoutConstraint!
    
    var timer: Timer? = nil
    var coreWrapper: CoreWrapper?
    var coreDelegate = CoreDelegate()

    override func windowDidLoad() {
        super.windowDidLoad()
        window!.delegate = self
        
        backgroundView.layer!.backgroundColor = CGColor(gray: 0, alpha: 1)
        
        let lowResNXDocument = document as! LowResNXDocument
        coreWrapper = lowResNXDocument.coreWrapper
        
        if let coreWrapper = coreWrapper {
            coreDelegate.context = UnsafeMutableRawPointer(Unmanaged.passUnretained(self).toOpaque())
            coreDelegate.interpreterDidFail = interpreterDidFail
            coreDelegate.diskDriveWillAccess = diskDriveWillAccess
            coreDelegate.diskDriveDidSave = diskDriveDidSave
            core_setDelegate(&coreWrapper.core, &coreDelegate)
            
            let secondsSincePowerOn = -(NSApp.delegate as! AppDelegate).launchDate.timeIntervalSinceNow
            core_willRunProgram(&coreWrapper.core, Int(secondsSincePowerOn))
            
            timer = Timer.scheduledTimer(timeInterval: 1.0/30.0, target: self, selector: #selector(LowResNXWindowController.update), userInfo: nil, repeats: true)
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
    
    func update() {
        if let coreWrapper = coreWrapper {
            core_update(&coreWrapper.core)
            lowResNXView.render(coreWrapper: coreWrapper)
        }
    }
    
    override func keyDown(with event: NSEvent) {
        guard let coreWrapper = coreWrapper else {
            return
        }
        
        switch event.keyCode {
        case 123:
            core_gamepadPressed(&coreWrapper.core, 0, GamepadButtonLeft)
        case 124:
            core_gamepadPressed(&coreWrapper.core, 0, GamepadButtonRight)
        case 125:
            core_gamepadPressed(&coreWrapper.core, 0, GamepadButtonDown)
        case 126:
            core_gamepadPressed(&coreWrapper.core, 0, GamepadButtonUp)
        case 6, 43: // Z, ,
            core_gamepadPressed(&coreWrapper.core, 0, GamepadButtonA)
        case 7, 47: // X, .
            core_gamepadPressed(&coreWrapper.core, 0, GamepadButtonB)
        case 2: // D
            core_gamepadPressed(&coreWrapper.core, 1, GamepadButtonLeft)
        case 5: // G
            core_gamepadPressed(&coreWrapper.core, 1, GamepadButtonRight)
        case 3: // F
            core_gamepadPressed(&coreWrapper.core, 1, GamepadButtonDown)
        case 15: // R
            core_gamepadPressed(&coreWrapper.core, 1, GamepadButtonUp)
        case 0: // A
            core_gamepadPressed(&coreWrapper.core, 1, GamepadButtonA)
        case 1: // S
            core_gamepadPressed(&coreWrapper.core, 1, GamepadButtonB)
        default:
            break
        }
        let characters = event.charactersIgnoringModifiers!
        if !characters.isEmpty {
            if characters == "\r" {
                core_returnPressed(&coreWrapper.core)
            } else if characters == "\u{7F}" {
                core_backspacePressed(&coreWrapper.core)
            } else {
                let text = characters.uppercased()
                let codes = text.unicodeScalars
                let key = codes[codes.startIndex]
                if key.value < 127 {
                    core_keyPressed(&coreWrapper.core, Int8(key.value))
                }
            }
        }
    }
    
    override func keyUp(with event: NSEvent) {
        guard let coreWrapper = coreWrapper else {
            return
        }
        
        switch event.keyCode {
        case 123:
            core_gamepadReleased(&coreWrapper.core, 0, GamepadButtonLeft)
        case 124:
            core_gamepadReleased(&coreWrapper.core, 0, GamepadButtonRight)
        case 125:
            core_gamepadReleased(&coreWrapper.core, 0, GamepadButtonDown)
        case 126:
            core_gamepadReleased(&coreWrapper.core, 0, GamepadButtonUp)
        case 6, 43: // Z, ,
            core_gamepadReleased(&coreWrapper.core, 0, GamepadButtonA)
        case 7, 47: // X, .
            core_gamepadReleased(&coreWrapper.core, 0, GamepadButtonB)
        case 2: // D
            core_gamepadReleased(&coreWrapper.core, 1, GamepadButtonLeft)
        case 5: // G
            core_gamepadReleased(&coreWrapper.core, 1, GamepadButtonRight)
        case 3: // F
            core_gamepadReleased(&coreWrapper.core, 1, GamepadButtonDown)
        case 15: // R
            core_gamepadReleased(&coreWrapper.core, 1, GamepadButtonUp)
        case 0: // A
            core_gamepadReleased(&coreWrapper.core, 1, GamepadButtonA)
        case 1: // S
            core_gamepadReleased(&coreWrapper.core, 1, GamepadButtonB)
        default:
            break
        }
    }
    
    override func mouseDragged(with event: NSEvent) {
        if let coreWrapper = coreWrapper {
            let point = screenPoint(event: event)
            core_touchDragged(&coreWrapper.core, Int32(point.x), Int32(point.y))
        }
    }
    
    override func mouseDown(with event: NSEvent) {
        if let coreWrapper = coreWrapper {
            let point = screenPoint(event: event)
            if point.y >= 0 {
                core_touchPressed(&coreWrapper.core, Int32(point.x), Int32(point.y))
            }
        }
    }
    
    override func mouseUp(with event: NSEvent) {
        if let coreWrapper = coreWrapper {
            core_touchReleased(&coreWrapper.core)
        }
    }
    
    func screenPoint(event: NSEvent) -> CGPoint {
        let point = event.locationInWindow
        let viewPoint = window!.contentView!.convert(point, to: lowResNXView)
        let x = viewPoint.x * CGFloat(SCREEN_WIDTH) / lowResNXView.bounds.size.width;
        let y = CGFloat(SCREEN_HEIGHT) - viewPoint.y * CGFloat(SCREEN_HEIGHT) / lowResNXView.bounds.size.height;
        return CGPoint(x: x, y: y);
    }

}

func interpreterDidFail(context: UnsafeMutableRawPointer?) -> Void {
    let windowController = Unmanaged<LowResNXWindowController>.fromOpaque(context!).takeUnretainedValue()
    let nxDocument = windowController.document as! LowResNXDocument
    
    if let coreWrapper = windowController.coreWrapper {
        windowController.presentError(nxDocument.getProgramError(errorCode: itp_getExitErrorCode(&coreWrapper.core)))
    }
}

func diskDriveWillAccess(context: UnsafeMutableRawPointer?) -> Void {
    let windowController = Unmanaged<LowResNXWindowController>.fromOpaque(context!).takeUnretainedValue()
    let nxDocument = windowController.document as! LowResNXDocument
    
    if let coreWrapper = windowController.coreWrapper {
        do {
            let diskURL = nxDocument.nxDiskURL()
            let data = try Data(contentsOf: diskURL)
            let success = data.withUnsafeBytes({ (chars: UnsafePointer<Int8>) -> Bool in
                disk_importDisk(&coreWrapper.core, chars)
            })
            if !success {
                //TODO
            }
        } catch {
            
        }
    }
}

func diskDriveDidSave(context: UnsafeMutableRawPointer?) -> Void {
    let windowController = Unmanaged<LowResNXWindowController>.fromOpaque(context!).takeUnretainedValue()
    let nxDocument = windowController.document as! LowResNXDocument
    
    if let coreWrapper = windowController.coreWrapper {
        let output = disk_exportDisk(&coreWrapper.core)
        if let output = output {
            let diskURL = nxDocument.nxDiskURL()
            let data = Data(bytes: output, count: Int(strlen(output)))
            do {
                try data.write(to: diskURL)
            } catch let error as NSError {
                windowController.presentError(error)
            }
            free(output);
        } else {
            //TODO
        }
    }
}
