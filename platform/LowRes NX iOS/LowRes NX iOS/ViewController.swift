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
    
    var coreDelegate = CoreDelegate()
    var displayLink: CADisplayLink?
    var coreWrapper: CoreWrapper?
    
    var pixelExactScaling: Bool = true {
        didSet {
            view.setNeedsLayout()
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()

        let delegate = UIApplication.shared.delegate as! AppDelegate
        coreWrapper = delegate.coreWrapper
        
        guard let coreWrapper = coreWrapper else {
            return
        }

        nxView.coreWrapper = coreWrapper
        
        coreDelegate.context = UnsafeMutableRawPointer(Unmanaged.passUnretained(self).toOpaque())
        coreDelegate.interpreterDidFail = interpreterDidFail
        coreDelegate.diskDriveWillAccess = diskDriveWillAccess
        coreDelegate.diskDriveDidSave = diskDriveDidSave
        coreDelegate.controlsDidChange = controlsDidChange
        core_setDelegate(&coreWrapper.core, &coreDelegate)
        
        core_willRunProgram(&coreWrapper.core, 0)
        
        let displayLink = CADisplayLink(target: self, selector: #selector(update))
        if #available(iOS 10.0, *) {
            displayLink.preferredFramesPerSecond = 30
        } else {
            displayLink.frameInterval = 2
        }
        self.displayLink = displayLink
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        displayLink!.add(to: .current, forMode: .defaultRunLoopMode)
    }
    
    override var prefersStatusBarHidden: Bool {
        return true
    }
    
    override func viewWillLayoutSubviews() {
        super.viewWillLayoutSubviews()
        let screenWidth = view.bounds.size.width
        let screenHeight = view.bounds.size.height
        var maxWidthFactor: CGFloat
        var maxHeightFactor: CGFloat
        if pixelExactScaling {
            let scale: CGFloat = view.window?.screen.scale ?? 1.0
            maxWidthFactor = floor(screenWidth * scale / CGFloat(SCREEN_WIDTH)) / scale
            maxHeightFactor = floor(screenHeight * scale / CGFloat(SCREEN_HEIGHT)) / scale
        } else {
            maxWidthFactor = screenWidth / CGFloat(SCREEN_WIDTH)
            maxHeightFactor = screenHeight / CGFloat(SCREEN_HEIGHT)
        }
        widthConstraint.constant = (maxWidthFactor < maxHeightFactor) ? maxWidthFactor * CGFloat(SCREEN_WIDTH) : maxHeightFactor * CGFloat(SCREEN_WIDTH)
    }
    
    func update(displaylink: CADisplayLink) {
        guard let coreWrapper = coreWrapper else {
            return
        }
        
        core_update(&coreWrapper.core)
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

func controlsDidChange(context: UnsafeMutableRawPointer?) -> Void {
//    let viewController = Unmanaged<ViewController>.fromOpaque(context!).takeUnretainedValue()
}
