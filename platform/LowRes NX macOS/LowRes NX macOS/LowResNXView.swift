//
//  LowResNXView.swift
//  LowRes NX macOS
//
//  Created by Timo Kloss on 29/4/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import Cocoa

func getBytePointerCallback(_ data: UnsafeMutableRawPointer?) -> UnsafeRawPointer? {
    return UnsafeRawPointer(data)
}

class LowResNXView: NSView {
    
    var core: UnsafeMutablePointer<Core>? = nil
    var data: UnsafeMutablePointer<UInt8>?
    var dataProvider: CGDataProvider?
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        
        let dataLength = Int(SCREEN_WIDTH) * Int(SCREEN_HEIGHT) * 4;
        data = UnsafeMutablePointer<UInt8>.allocate(capacity: dataLength)
        var callbacks = CGDataProviderDirectCallbacks(version: 0, getBytePointer: getBytePointerCallback, releaseBytePointer: nil, getBytesAtPosition: nil, releaseInfo: nil)
        dataProvider = CGDataProvider(directInfo: data, size: off_t(dataLength), callbacks: &callbacks)
    }
    
    func render() {
        if let dataProvider = dataProvider {
            if core != nil {
                video_renderScreen(core, data, SCREEN_WIDTH*4)
            }
            let image = CGImage(width: Int(SCREEN_WIDTH), height: Int(SCREEN_HEIGHT), bitsPerComponent: 8, bitsPerPixel: 32, bytesPerRow: Int(SCREEN_WIDTH)*4, space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.noneSkipLast.rawValue), provider: dataProvider, decode: nil, shouldInterpolate: false, intent: .defaultIntent)
            
            layer?.contents = image
            layer?.magnificationFilter = kCAFilterNearest
        }
    }
    
}
