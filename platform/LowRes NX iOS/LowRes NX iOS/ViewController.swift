//
//  ViewController.swift
//  LowRes NX iOS
//
//  Created by Timo Kloss on 1/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import UIKit
import GameController

class ViewController: UIViewController, UIKeyInput {
    
    @IBOutlet weak var nxView: LowResNXView!
    @IBOutlet weak var containerView: UIView!
    @IBOutlet weak var widthConstraint: NSLayoutConstraint!
    @IBOutlet weak var keyboardConstraint: NSLayoutConstraint!
    
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
        configureGameControllers()
        
        let recognizer = UITapGestureRecognizer(target: self, action: #selector(handleTap))
        view.addGestureRecognizer(recognizer)
        
        let displayLink = CADisplayLink(target: self, selector: #selector(update))
        self.displayLink = displayLink
        
        NotificationCenter.default.addObserver(self, selector: #selector(keyboardWillShow), name: .UIKeyboardWillShow, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(keyboardWillHide), name: .UIKeyboardWillHide, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(controllerDidConnect), name: .GCControllerDidConnect, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(controllerDidDisconnect), name: .GCControllerDidDisconnect, object: nil)
    }
    
    deinit {
        NotificationCenter.default.removeObserver(self)
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        displayLink!.add(to: .current, forMode: .defaultRunLoopMode)
    }
    
    override var prefersStatusBarHidden: Bool {
        return true
    }
    
    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        let screenWidth = containerView.bounds.size.width
        let screenHeight = containerView.bounds.size.height
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
    
    @objc func update(displaylink: CADisplayLink) {
        updateGameControllers()
        
        if let coreWrapper = coreWrapper {
            core_update(&coreWrapper.core)
            nxView.render()
        }
    }
    
    func configureGameControllers() {
        if let coreWrapper = coreWrapper {
            let gameControllers = GCController.controllers()
            
            core_setNumPhysicalGamepads(&coreWrapper.core, Int32(gameControllers.count))
            
            var count = 0
            for gameController in gameControllers {
                gameController.playerIndex = GCControllerPlayerIndex(rawValue: count)!
                gameController.controllerPausedHandler = { [weak self] (controller) in
                    if let coreWrapper = self?.coreWrapper {
                        core_pausePressed(&coreWrapper.core)
                    }
                }
                count += 1
            }
        }
    }
    
    func updateGameControllers() {
        if let coreWrapper = coreWrapper {
            for gameController in GCController.controllers() {
                if let gamepad = gameController.gamepad, gameController.playerIndex != .indexUnset {
                    var up = gamepad.dpad.up.isPressed
                    var down = gamepad.dpad.down.isPressed
                    var left = gamepad.dpad.left.isPressed
                    var right = gamepad.dpad.right.isPressed
                    if let stick = gameController.extendedGamepad?.leftThumbstick {
                        up = up || stick.up.isPressed
                        down = down || stick.down.isPressed
                        left = left || stick.left.isPressed
                        right = right || stick.right.isPressed
                    }
                    let buttonA = gamepad.buttonA.isPressed || gamepad.buttonX.isPressed
                    let buttonB = gamepad.buttonB.isPressed || gamepad.buttonY.isPressed
                    core_setGamepad(&coreWrapper.core, Int32(gameController.playerIndex.rawValue), up, down, left, right, buttonA, buttonB)
                }
            }
        }
    }
    
    @objc func handleTap(sender: UITapGestureRecognizer) {
        if let coreWrapper = coreWrapper {
            if sender.state == .ended {
                if core_getKeyboardEnabled(&coreWrapper.core) {
                    becomeFirstResponder()
                }
            }
        }
    }
    
    override var canBecomeFirstResponder: Bool {
        if let coreWrapper = coreWrapper {
            return core_getKeyboardEnabled(&coreWrapper.core)
        }
        return false
    }
    
    @objc func keyboardWillShow(_ notification: NSNotification) {
        if let frameValue = notification.userInfo?[UIKeyboardFrameEndUserInfoKey] as? NSValue {
            let frame = frameValue.cgRectValue
            keyboardConstraint.constant = view.bounds.size.height - frame.origin.y
            UIView.animate(withDuration: 0.3, animations: { 
                self.view.layoutIfNeeded()
            })
        }
    }

    @objc func keyboardWillHide(_ notification: NSNotification) {
        keyboardConstraint.constant = 0
        UIView.animate(withDuration: 0.3, animations: {
            self.view.layoutIfNeeded()
        })
    }
    
    @objc func controllerDidConnect(_ notification: NSNotification) {
        configureGameControllers()
    }

    @objc func controllerDidDisconnect(_ notification: NSNotification) {
        configureGameControllers()
    }
    
    // MARK: - Core Delegate
    
    func coreInterpreterDidFail(coreError: CoreError) {
    }
    
    func coreDiskDriveWillAccess(diskDataManager: UnsafeMutablePointer<DataManager>?) -> Bool {
        return true
    }
    
    func coreDiskDriveDidSave(diskDataManager: UnsafeMutablePointer<DataManager>?) {
    }
    
    func coreControlsDidChange(controlsInfo: ControlsInfo) {
        if controlsInfo.isKeyboardEnabled {
            becomeFirstResponder()
        } else {
            resignFirstResponder()
        }
    }
    
    // MARK: - UIKeyInput
    
    var autocorrectionType: UITextAutocorrectionType = .no
    var spellCheckingType: UITextSpellCheckingType = .no
    var keyboardAppearance: UIKeyboardAppearance = .dark
    
    var hasText: Bool {
        return true
    }
    
    func insertText(_ text: String) {
        if let coreWrapper = coreWrapper {
            if text == "\n" {
                core_returnPressed(&coreWrapper.core)
            } else if let key = text.uppercased().unicodeScalars.first?.value {
                if key < 127 {
                    core_keyPressed(&coreWrapper.core, Int8(key))
                }
            }
        }
    }
    
    func deleteBackward() {
        if let coreWrapper = coreWrapper {
            core_backspacePressed(&coreWrapper.core)
        }
    }
    
    // this is from UITextInput, needed because of crash on iPhone 6 keyboard (left/right arrows)
    var selectedTextRange: UITextRange? {
        return nil
    }
    
}

func interpreterDidFail(context: UnsafeMutableRawPointer?, coreError: CoreError) -> Void {
    let viewController = Unmanaged<ViewController>.fromOpaque(context!).takeUnretainedValue()
    viewController.coreInterpreterDidFail(coreError: coreError)
}

func diskDriveWillAccess(context: UnsafeMutableRawPointer?, diskDataManager: UnsafeMutablePointer<DataManager>?) -> Bool {
    let viewController = Unmanaged<ViewController>.fromOpaque(context!).takeUnretainedValue()
    return viewController.coreDiskDriveWillAccess(diskDataManager: diskDataManager)
}

func diskDriveDidSave(context: UnsafeMutableRawPointer?, diskDataManager: UnsafeMutablePointer<DataManager>?) -> Void {
    let viewController = Unmanaged<ViewController>.fromOpaque(context!).takeUnretainedValue()
    viewController.coreDiskDriveDidSave(diskDataManager: diskDataManager)
}

func controlsDidChange(context: UnsafeMutableRawPointer?, controlsInfo: ControlsInfo) -> Void {
    let viewController = Unmanaged<ViewController>.fromOpaque(context!).takeUnretainedValue()
    viewController.coreControlsDidChange(controlsInfo: controlsInfo)
}
