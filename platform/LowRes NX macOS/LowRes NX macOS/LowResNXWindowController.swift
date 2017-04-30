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
    
    var timer: Timer? = nil
    var core: UnsafeMutablePointer<Core>? = nil

    override func windowDidLoad() {
        super.windowDidLoad()
        window!.delegate = self
        window!.acceptsMouseMovedEvents = true
        
        let lowResNXDocument = document as! LowResNXDocument
        core = UnsafeMutablePointer<Core>(&lowResNXDocument.core)
        
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
    
    override func mouseMoved(with event: NSEvent) {
        let point = event.locationInWindow
        let viewPoint = lowResNXView.convert(point, to: nil)
        let x = viewPoint.x * CGFloat(SCREEN_WIDTH) / lowResNXView.bounds.size.width;
        let y = CGFloat(SCREEN_HEIGHT) - viewPoint.y * CGFloat(SCREEN_HEIGHT) / lowResNXView.bounds.size.height;
        core_mouseMoved(core, Int32(x), Int32(y))
    }
    
    override func mouseDragged(with event: NSEvent) {
        mouseMoved(with: event)
    }
    
    override func mouseDown(with event: NSEvent) {
        core_mouseDown(core)
    }
    
    override func mouseUp(with event: NSEvent) {
        core_mouseUp(core)
    }

}
