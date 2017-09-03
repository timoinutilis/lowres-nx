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
    @IBOutlet var twoPlayerView: UIView!
    @IBOutlet weak var playerGamepad: Gamepad!
    @IBOutlet weak var playerButtonA: UIButton!
    @IBOutlet weak var playerButtonB: UIButton!
    @IBOutlet weak var player1Gamepad: Gamepad!
    @IBOutlet weak var player1ButtonA: UIButton!
    @IBOutlet weak var player1ButtonB: UIButton!
    @IBOutlet weak var player2Gamepad: Gamepad!
    @IBOutlet weak var player2ButtonA: UIButton!
    @IBOutlet weak var player2ButtonB: UIButton!
    
    var coreDelegate = CoreDelegate()
    var displayLink: CADisplayLink?
    var coreWrapper: CoreWrapper?
    
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
        displayLink.add(to: .current, forMode: .defaultRunLoopMode)
        self.displayLink = displayLink
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
    
    func updateControlsUI() {
        guard let coreWrapper = coreWrapper else {
            return
        }
        
        onePlayerView.removeFromSuperview()
        twoPlayerView.removeFromSuperview()
        
        let gamepads = core_getGamepadsEnabled(&coreWrapper.core)
        if gamepads == 1 {
            addBottomView(onePlayerView)
        } else if gamepads == 2 {
            addBottomView(twoPlayerView)
        }
    }
    
    private func addBottomView(_ subview: UIView) {
        subview.translatesAutoresizingMaskIntoConstraints = false
        view.addSubview(subview)
        let bottom = NSLayoutConstraint(item: view, attribute: .bottom, relatedBy: .equal, toItem: subview, attribute: .bottom, multiplier: 1, constant: 0)
        let left = NSLayoutConstraint(item: view, attribute: .left, relatedBy: .equal, toItem: subview, attribute: .left, multiplier: 1, constant: 0)
        let right = NSLayoutConstraint(item: view, attribute: .right, relatedBy: .equal, toItem: subview, attribute: .right, multiplier: 1, constant: 0)
        bottom.isActive = true
        left.isActive = true
        right.isActive = true
    }
    
    func update(displaylink: CADisplayLink) {
        guard let coreWrapper = coreWrapper else {
            return
        }
        
        let gamepads = core_getGamepadsEnabled(&coreWrapper.core)
        if gamepads == 1 {
            core_setGamepad(&coreWrapper.core, 0, playerGamepad.isDirUp, playerGamepad.isDirDown, playerGamepad.isDirLeft, playerGamepad.isDirRight, playerButtonA.isHighlighted, playerButtonB.isHighlighted)
        } else if gamepads == 2 {
            core_setGamepad(&coreWrapper.core, 0, player1Gamepad.isDirUp, player1Gamepad.isDirDown, player1Gamepad.isDirLeft, player1Gamepad.isDirRight, player1ButtonA.isHighlighted, player1ButtonB.isHighlighted)
            core_setGamepad(&coreWrapper.core, 1, player2Gamepad.isDirUp, player2Gamepad.isDirDown, player2Gamepad.isDirLeft, player2Gamepad.isDirRight, player2ButtonA.isHighlighted, player2ButtonB.isHighlighted)
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
    let viewController = Unmanaged<ViewController>.fromOpaque(context!).takeUnretainedValue()
    viewController.updateControlsUI()
}
