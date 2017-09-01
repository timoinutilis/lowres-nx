//
//  ViewController.swift
//  LowRes NX iOS
//
//  Created by Timo Kloss on 1/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import UIKit

class ViewController: UIViewController {
    
    @IBOutlet weak var nxView: LowResNXView!
    @IBOutlet weak var widthConstraint: NSLayoutConstraint!
    
    var core: UnsafeMutablePointer<Core>? = nil
    var coreDelegate = CoreDelegate()
    var displayLink: CADisplayLink?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let delegate = UIApplication.shared.delegate as! AppDelegate
        core = UnsafeMutablePointer<Core>(&delegate.core)
        
        nxView.core = core
        
        coreDelegate.context = UnsafeMutableRawPointer(Unmanaged.passUnretained(self).toOpaque())
        coreDelegate.interpreterDidFail = interpreterDidFail
        coreDelegate.diskDriveWillAccess = diskDriveWillAccess
        coreDelegate.diskDriveDidSave = diskDriveDidSave
        core_setDelegate(core, &coreDelegate)
        
        core_willRunProgram(core, 0)
                
        let displayLink = CADisplayLink(target: self, selector: #selector(update))
        if #available(iOS 10.0, *) {
            displayLink.preferredFramesPerSecond = 30
        } else {
            displayLink.frameInterval = 2
        }
        displayLink.add(to: .current, forMode: .defaultRunLoopMode)
        self.displayLink = displayLink
    }
    
    override var prefersStatusBarHidden: Bool {
        return true
    }
    
    override func viewWillLayoutSubviews() {
        super.viewWillLayoutSubviews()
        let width = view.bounds.size.width
        widthConstraint.constant = floor(width / CGFloat(SCREEN_WIDTH)) * CGFloat(SCREEN_WIDTH)
    }
    
    func update(displaylink: CADisplayLink) {
        core_update(core)
        nxView.render()
    }
    
}

func interpreterDidFail(context: UnsafeMutableRawPointer?) -> Void {
//    let viewController = Unmanaged<ViewController>.fromOpaque(context!).takeUnretainedValue()
}

func diskDriveWillAccess(context: UnsafeMutableRawPointer?) -> Void {
//    let viewController = Unmanaged<ViewController>.fromOpaque(context!).takeUnretainedValue()
}

func diskDriveDidSave(context: UnsafeMutableRawPointer?) -> Void {
//    let viewController = Unmanaged<ViewController>.fromOpaque(context!).takeUnretainedValue()
}
