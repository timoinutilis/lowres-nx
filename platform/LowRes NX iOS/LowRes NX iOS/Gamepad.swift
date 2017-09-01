//
//  Gamepad.swift
//  LowRes NX iOS
//
//  Created by Timo Kloss on 1/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import UIKit

class Gamepad: UIControl {
    
    enum Image: Int {
        case normal
        case up
        case upRight
        case right
        case downRight
        case down
        case downLeft
        case left
        case upLeft
    }
    
    var isDirUp = false
    var isDirDown = false
    var isDirLeft = false
    var isDirRight = false
    
    private var imageView: UIImageView
    
    private var images: [UIImage] = [UIImage(named:"joypad")!,
                                     UIImage(named:"joypad_pressed_u")!,
                                     UIImage(named:"joypad_pressed_ur")!,
                                     UIImage(named:"joypad_pressed_r")!,
                                     UIImage(named:"joypad_pressed_dr")!,
                                     UIImage(named:"joypad_pressed_d")!,
                                     UIImage(named:"joypad_pressed_dl")!,
                                     UIImage(named:"joypad_pressed_l")!,
                                     UIImage(named:"joypad_pressed_ul")!]
    
    required init?(coder aDecoder: NSCoder) {
        imageView = UIImageView(image: images[0])
        super.init(coder: aDecoder)
        addSubview(imageView)
        backgroundColor = UIColor.clear
    }
    
    override var intrinsicContentSize: CGSize {
        return images.first!.size
    }
    
    override func hitTest(_ point: CGPoint, with event: UIEvent?) -> UIView? {
        if !isHidden && isUserInteractionEnabled {
            let errorMargin: CGFloat = 40;
            let largerFrame = CGRect(x: -errorMargin, y: -errorMargin, width: frame.size.width + 2 * errorMargin, height: frame.size.height + 2 * errorMargin)
            return largerFrame.contains(point) ? self : nil
        }
        return nil
    }
    
    override func beginTracking(_ touch: UITouch, with event: UIEvent?) -> Bool {
        let begin = super.beginTracking(touch, with: event)
        if begin {
            updateDirections(touch: touch)
        }
        return begin
    }
    
    override func continueTracking(_ touch: UITouch, with event: UIEvent?) -> Bool {
        let cont = super.continueTracking(touch, with: event)
        if cont {
            updateDirections(touch: touch)
        }
        return cont
    }
    
    override func endTracking(_ touch: UITouch?, with event: UIEvent?) {
        resetDirections()
        super.endTracking(touch, with: event)
    }
    
    override func cancelTracking(with event: UIEvent?) {
        resetDirections()
        super.cancelTracking(with: event)
    }
    
    private func updateDirections(touch: UITouch) {
        var point = touch.location(in: self)
        point.x -= bounds.size.width * 0.5;
        point.y -= bounds.size.height * 0.5;
        isDirUp = (point.y < -20.0) && abs(point.x / point.y) < 2.0;
        isDirDown = (point.y > 20.0) && abs(point.x / point.y) < 2.0;
        isDirLeft = (point.x < -20.0) && abs(point.y / point.x) < 2.0;
        isDirRight = (point.x > 20.0) && abs(point.y / point.x) < 2.0;
        updateImage()
    }
    
    private func resetDirections() {
        isDirUp = false
        isDirDown = false
        isDirLeft = false
        isDirRight = false
        updateImage()
    }
    
    private func updateImage() {
        var gi = Image.normal
        if isDirUp {
            if isDirLeft {
                gi = .upLeft
            } else if isDirRight {
                gi = .upRight
            } else {
                gi = .up
            }
        } else if isDirDown {
            if isDirLeft {
                gi = .downLeft
            } else if isDirRight {
                gi = .downRight
            } else {
                gi = .down
            }
        } else if isDirLeft {
            gi = .left
        } else if isDirRight {
            gi = .right
        }
    
        imageView.image = images[gi.rawValue]
    }
    
}
