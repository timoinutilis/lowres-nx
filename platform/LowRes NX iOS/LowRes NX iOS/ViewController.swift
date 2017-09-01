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
    
    @IBOutlet var onePlayerView: UIView!
    @IBOutlet weak var player1Gamepad: Gamepad!
    @IBOutlet weak var player1ButtonA: UIButton!
    @IBOutlet weak var player1ButtonB: UIButton!
    
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
        
        addOnePlayerView()
    }
    
    override var prefersStatusBarHidden: Bool {
        return true
    }
    
    override func viewWillLayoutSubviews() {
        super.viewWillLayoutSubviews()
        let screenWidth = view.bounds.size.width
        let screenHeight = view.bounds.size.height
        let maxWidthFactor = floor(screenWidth / CGFloat(SCREEN_WIDTH))
        let maxHeightFactor = floor(screenHeight / CGFloat(SCREEN_HEIGHT))
        widthConstraint.constant = (maxWidthFactor < maxHeightFactor) ? maxWidthFactor * CGFloat(SCREEN_WIDTH) : maxHeightFactor * CGFloat(SCREEN_WIDTH)
    }
    
    private func addOnePlayerView() {
        onePlayerView.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(onePlayerView)
        let bottom = NSLayoutConstraint(item: view, attribute: .bottom, relatedBy: .equal, toItem: onePlayerView, attribute: .bottom, multiplier: 1, constant: 44)
        let left = NSLayoutConstraint(item: view, attribute: .left, relatedBy: .equal, toItem: onePlayerView, attribute: .left, multiplier: 1, constant: 0)
        let right = NSLayoutConstraint(item: view, attribute: .right, relatedBy: .equal, toItem: onePlayerView, attribute: .right, multiplier: 1, constant: 0)
        bottom.isActive = true
        left.isActive = true
        right.isActive = true
    }
    
    func update(displaylink: CADisplayLink) {
        core_setGamepad(core, 0, player1Gamepad.isDirUp, player1Gamepad.isDirDown, player1Gamepad.isDirLeft, player1Gamepad.isDirRight, player1ButtonA.isHighlighted, player1ButtonB.isHighlighted)
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
