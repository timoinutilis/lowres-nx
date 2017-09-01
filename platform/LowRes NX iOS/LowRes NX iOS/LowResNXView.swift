//
//  LowResNXView.swift
//  LowRes NX iOS
//
//  Created by Timo Kloss on 1/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import UIKit

class LowResNXView: UIView {
    var core: UnsafeMutablePointer<Core>? = nil

    private var data: UnsafeMutablePointer<UInt8>?
    private var dataProvider: CGDataProvider?
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        
        let dataLength = Int(SCREEN_WIDTH) * Int(SCREEN_HEIGHT) * 4;
        data = UnsafeMutablePointer<UInt8>.allocate(capacity: dataLength)
        var callbacks = CGDataProviderDirectCallbacks(version: 0, getBytePointer: getBytePointerCallback, releaseBytePointer: nil, getBytesAtPosition: nil, releaseInfo: nil)
        dataProvider = CGDataProvider(directInfo: data, size: off_t(dataLength), callbacks: &callbacks)
    }
    
    func render() {
        if let dataProvider = dataProvider {
            video_renderScreen(core, data, SCREEN_WIDTH*4)
            let image = CGImage(width: Int(SCREEN_WIDTH), height: Int(SCREEN_HEIGHT), bitsPerComponent: 8, bitsPerPixel: 32, bytesPerRow: Int(SCREEN_WIDTH)*4, space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.noneSkipLast.rawValue), provider: dataProvider, decode: nil, shouldInterpolate: false, intent: .defaultIntent)
            
            layer.contents = image
            layer.magnificationFilter = kCAFilterNearest
        }
    }
    
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        if let touch = touches.first {
            let point = screenPoint(touch: touch)
            core_touchPressed(core, Int32(point.x), Int32(point.y))
        }
    }
    
    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        if let touch = touches.first {
            let point = screenPoint(touch: touch)
            core_touchDragged(core, Int32(point.x), Int32(point.y))
        }
    }
    
    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        core_touchReleased(core)
    }
    
    override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
        core_touchReleased(core)
    }
    
    private func screenPoint(touch: UITouch) -> CGPoint {
        let viewPoint = touch.location(in: self)
        let x = viewPoint.x * CGFloat(SCREEN_WIDTH) / bounds.size.width;
        let y = viewPoint.y * CGFloat(SCREEN_HEIGHT) / bounds.size.height;
        return CGPoint(x: x, y: y);
    }

}

func getBytePointerCallback(_ data: UnsafeMutableRawPointer?) -> UnsafeRawPointer? {
    return UnsafeRawPointer(data)
}
