//
//  ViewController.swift
//  LowRes NX iOS
//
//  Created by Timo Kloss on 1/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import UIKit

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
        
        let recognizer = UITapGestureRecognizer(target: self, action: #selector(handleTap))
        view.addGestureRecognizer(recognizer)
        
        let displayLink = CADisplayLink(target: self, selector: #selector(update))
        if #available(iOS 10.0, *) {
            displayLink.preferredFramesPerSecond = 30
        } else {
            displayLink.frameInterval = 2
        }
        self.displayLink = displayLink
        
        NotificationCenter.default.addObserver(self, selector: #selector(keyboardWillShow), name: .UIKeyboardWillShow, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(keyboardWillHide), name: .UIKeyboardWillHide, object: nil)
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
    
    func update(displaylink: CADisplayLink) {
        if let coreWrapper = coreWrapper {
            core_update(&coreWrapper.core)
            nxView.render()
        }
    }
    
    func handleTap(sender: UITapGestureRecognizer) {
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
    
    func keyboardWillShow(_ notification: NSNotification) {
        if let frameValue = notification.userInfo?[UIKeyboardFrameEndUserInfoKey] as? NSValue {
            let frame = frameValue.cgRectValue
            keyboardConstraint.constant = view.bounds.size.height - frame.origin.y
            UIView.animate(withDuration: 0.3, animations: { 
                self.view.layoutIfNeeded()
            })
        }
    }

    func keyboardWillHide(_ notification: NSNotification) {
        keyboardConstraint.constant = 0
        UIView.animate(withDuration: 0.3, animations: {
            self.view.layoutIfNeeded()
        })
    }
    
    // MARK: - Core Delegate
    
    func coreInterpreterDidFail() {
    }
    
    func coreDiskDriveWillAccess() {
    }
    
    func coreDiskDriveDidSave() {
    }
    
    func coreControlsDidChange() {
        if let coreWrapper = coreWrapper {
            if core_getKeyboardEnabled(&coreWrapper.core) {
                becomeFirstResponder()
            } else {
                resignFirstResponder()
            }
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

func interpreterDidFail(context: UnsafeMutableRawPointer?) -> Void {
    let viewController = Unmanaged<ViewController>.fromOpaque(context!).takeUnretainedValue()
    viewController.coreInterpreterDidFail()
}

func diskDriveWillAccess(context: UnsafeMutableRawPointer?) -> Void {
    let viewController = Unmanaged<ViewController>.fromOpaque(context!).takeUnretainedValue()
    viewController.coreDiskDriveWillAccess()
}

func diskDriveDidSave(context: UnsafeMutableRawPointer?) -> Void {
    let viewController = Unmanaged<ViewController>.fromOpaque(context!).takeUnretainedValue()
    viewController.coreDiskDriveDidSave()
}

func controlsDidChange(context: UnsafeMutableRawPointer?) -> Void {
    let viewController = Unmanaged<ViewController>.fromOpaque(context!).takeUnretainedValue()
    viewController.coreControlsDidChange()
}
