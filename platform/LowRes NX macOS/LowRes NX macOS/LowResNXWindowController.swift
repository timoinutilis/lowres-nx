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
    
    var timer: Timer? = nil
    var core: UnsafeMutablePointer<Core>? = nil
    var coreDelegate = CoreDelegate()

    override func windowDidLoad() {
        super.windowDidLoad()
        window!.delegate = self
        
        backgroundView.layer!.backgroundColor = CGColor(gray: 0, alpha: 1)
        
        let lowResNXDocument = document as! LowResNXDocument
        core = UnsafeMutablePointer<Core>(&lowResNXDocument.core)
        
        coreDelegate.context = UnsafeMutableRawPointer(Unmanaged.passUnretained(self).toOpaque())
        coreDelegate.interpreterDidFail = interpreterDidFail
        coreDelegate.diskDriveWillAccess = diskDriveWillAccess
        coreDelegate.diskDriveDidSave = diskDriveDidSave
        core_setDelegate(core, &coreDelegate)
        
        timer = Timer.scheduledTimer(timeInterval: 1.0/30.0, target: self, selector: #selector(LowResNXWindowController.update), userInfo: nil, repeats: true)

    }
    
    func windowWillClose(_ notification: Notification) {
        timer?.invalidate()
    }
    
    func update() {
        core_update(core)
        lowResNXView.render(core: core!)
    }
    
    override func keyDown(with event: NSEvent) {
        switch event.keyCode {
        case 123:
            core_gamepadPressed(core, 0, GamepadButtonLeft)
        case 124:
            core_gamepadPressed(core, 0, GamepadButtonRight)
        case 125:
            core_gamepadPressed(core, 0, GamepadButtonDown)
        case 126:
            core_gamepadPressed(core, 0, GamepadButtonUp)
        case 47:
            core_gamepadPressed(core, 0, GamepadButtonA)
        case 44:
            core_gamepadPressed(core, 0, GamepadButtonB)
        case 2:
            core_gamepadPressed(core, 1, GamepadButtonLeft)
        case 5:
            core_gamepadPressed(core, 1, GamepadButtonRight)
        case 3:
            core_gamepadPressed(core, 1, GamepadButtonDown)
        case 15:
            core_gamepadPressed(core, 1, GamepadButtonUp)
        case 0:
            core_gamepadPressed(core, 1, GamepadButtonA)
        case 1:
            core_gamepadPressed(core, 1, GamepadButtonB)
        default:
            break
        }
        let characters = event.charactersIgnoringModifiers!
        if !characters.isEmpty {
            if characters == "\r" {
                core_returnPressed(core)
            } else if characters == "\u{7F}" {
                core_backspacePressed(core)
            } else {
                let text = characters.uppercased()
                let codes = text.unicodeScalars
                let key = codes[codes.startIndex]
                if key.value < 127 {
                    core_keyPressed(core, Int8(key.value))
                }
            }
        }
    }
    
    override func keyUp(with event: NSEvent) {
        switch event.keyCode {
        case 123:
            core_gamepadReleased(core, 0, GamepadButtonLeft)
        case 124:
            core_gamepadReleased(core, 0, GamepadButtonRight)
        case 125:
            core_gamepadReleased(core, 0, GamepadButtonDown)
        case 126:
            core_gamepadReleased(core, 0, GamepadButtonUp)
        case 47:
            core_gamepadReleased(core, 0, GamepadButtonA)
        case 44:
            core_gamepadReleased(core, 0, GamepadButtonB)
        case 2:
            core_gamepadReleased(core, 1, GamepadButtonLeft)
        case 5:
            core_gamepadReleased(core, 1, GamepadButtonRight)
        case 3:
            core_gamepadReleased(core, 1, GamepadButtonDown)
        case 15:
            core_gamepadReleased(core, 1, GamepadButtonUp)
        case 0:
            core_gamepadReleased(core, 1, GamepadButtonA)
        case 1:
            core_gamepadReleased(core, 1, GamepadButtonB)
        default:
            break
        }
    }
    
    override func mouseDragged(with event: NSEvent) {
        let point = screenPoint(event: event)
        core_touchDragged(core, Int32(point.x), Int32(point.y))
    }
    
    override func mouseDown(with event: NSEvent) {
        let point = screenPoint(event: event)
        if point.y >= 0 {
            core_touchPressed(core, Int32(point.x), Int32(point.y))
        }
    }
    
    override func mouseUp(with event: NSEvent) {
        core_touchReleased(core)
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
    
    windowController.presentError(nxDocument.getProgramError(errorCode: itp_getExitErrorCode(windowController.core)))
}

func diskDriveWillAccess(context: UnsafeMutableRawPointer?) -> Void {
    let windowController = Unmanaged<LowResNXWindowController>.fromOpaque(context!).takeUnretainedValue()
    let nxDocument = windowController.document as! LowResNXDocument
    
    do {
        let diskURL = nxDocument.nxDiskURL()
        let data = try Data(contentsOf: diskURL)
        let success = data.withUnsafeBytes({ (chars: UnsafePointer<Int8>) -> Bool in
            disk_importDisk(windowController.core, chars)
        })
        if !success {
            //TODO
        }
    } catch {
        
    }
}

func diskDriveDidSave(context: UnsafeMutableRawPointer?) -> Void {
    let windowController = Unmanaged<LowResNXWindowController>.fromOpaque(context!).takeUnretainedValue()
    let nxDocument = windowController.document as! LowResNXDocument
    
    let output = disk_exportDisk(windowController.core)
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
