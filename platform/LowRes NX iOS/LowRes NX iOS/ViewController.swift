//
//  ViewController.swift
//  LowRes NX iOS
//
//  Created by Timo Kloss on 1/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import UIKit

class ViewController: UIViewController {
    
    @IBOutlet weak var nxView: UIView!
    @IBOutlet weak var widthConstraint: NSLayoutConstraint!
    
    var core: UnsafeMutablePointer<Core>? = nil
    var coreDelegate = CoreDelegate()
    var data: UnsafeMutablePointer<UInt8>?
    var dataProvider: CGDataProvider?
    var displayLink: CADisplayLink?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let delegate = UIApplication.shared.delegate as! AppDelegate
        core = UnsafeMutablePointer<Core>(&delegate.core)
        
        coreDelegate.context = UnsafeMutableRawPointer(Unmanaged.passUnretained(self).toOpaque())
        coreDelegate.interpreterDidFail = interpreterDidFail
        coreDelegate.diskDriveWillAccess = diskDriveWillAccess
        coreDelegate.diskDriveDidSave = diskDriveDidSave
        core_setDelegate(core, &coreDelegate)
        
        core_willRunProgram(core, 0)
        
        let dataLength = Int(SCREEN_WIDTH) * Int(SCREEN_HEIGHT) * 4;
        data = UnsafeMutablePointer<UInt8>.allocate(capacity: dataLength)
        var callbacks = CGDataProviderDirectCallbacks(version: 0, getBytePointer: getBytePointerCallback, releaseBytePointer: nil, getBytesAtPosition: nil, releaseInfo: nil)
        dataProvider = CGDataProvider(directInfo: data, size: off_t(dataLength), callbacks: &callbacks)
        
        let displayLink = CADisplayLink(target: self, selector: #selector(update))
        displayLink.preferredFramesPerSecond = 30
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
        render()
    }
    
    func render() {
        if let dataProvider = dataProvider {
            video_renderScreen(core, data, SCREEN_WIDTH*4)
            let image = CGImage(width: Int(SCREEN_WIDTH), height: Int(SCREEN_HEIGHT), bitsPerComponent: 8, bitsPerPixel: 32, bytesPerRow: Int(SCREEN_WIDTH)*4, space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.noneSkipLast.rawValue), provider: dataProvider, decode: nil, shouldInterpolate: false, intent: .defaultIntent)
            
            nxView.layer.contents = image
            nxView.layer.magnificationFilter = kCAFilterNearest
        }
    }

}

func getBytePointerCallback(_ data: UnsafeMutableRawPointer?) -> UnsafeRawPointer? {
    return UnsafeRawPointer(data)
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
