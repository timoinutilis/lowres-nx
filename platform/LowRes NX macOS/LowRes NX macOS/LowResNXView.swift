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
    
    let screenshotScaleFactor = 3
    
    var data: UnsafeMutablePointer<UInt8>?
    var dataProvider: CGDataProvider?
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        
        let dataLength = Int(SCREEN_WIDTH) * Int(SCREEN_HEIGHT) * 4;
        data = UnsafeMutablePointer<UInt8>.allocate(capacity: dataLength)
        var callbacks = CGDataProviderDirectCallbacks(version: 0, getBytePointer: getBytePointerCallback, releaseBytePointer: nil, getBytesAtPosition: nil, releaseInfo: nil)
        dataProvider = CGDataProvider(directInfo: data, size: off_t(dataLength), callbacks: &callbacks)
    }
    
    func render(coreWrapper: CoreWrapper) {
        if let dataProvider = dataProvider {
            video_renderScreen(&coreWrapper.core, data, SCREEN_WIDTH*4)
            let image = CGImage(width: Int(SCREEN_WIDTH), height: Int(SCREEN_HEIGHT), bitsPerComponent: 8, bitsPerPixel: 32, bytesPerRow: Int(SCREEN_WIDTH)*4, space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.noneSkipLast.rawValue), provider: dataProvider, decode: nil, shouldInterpolate: false, intent: .defaultIntent)
            
            layer?.contents = image
            layer?.magnificationFilter = kCAFilterNearest
        }
    }
    
    func capturePNG() -> Data? {
        if let contents = layer?.contents {
            let cgImage = contents as! CGImage
            
            let imageWidth = Int(SCREEN_WIDTH) * screenshotScaleFactor
            let imageHeight = Int(SCREEN_HEIGHT) * screenshotScaleFactor
            
            guard
                let colorSpace = cgImage.colorSpace,
                let context = CGContext(data: nil, width: imageWidth, height: imageHeight, bitsPerComponent: cgImage.bitsPerComponent, bytesPerRow: cgImage.bytesPerRow * screenshotScaleFactor, space: colorSpace, bitmapInfo: cgImage.alphaInfo.rawValue)
                else { return nil }
            
            // draw image to context (resizing it)
            context.interpolationQuality = .none
            context.draw(cgImage, in: CGRect(x: 0, y: 0, width: imageWidth, height: imageHeight))
            
            // extract resulting image from context
            guard let scaledImage = context.makeImage() else { return nil }
            
            let nsImage = NSImage(cgImage: scaledImage, size: NSSize(width: imageWidth, height: imageHeight))
            
            guard
                let imgData: Data = nsImage.tiffRepresentation,
                let bitmap: NSBitmapImageRep = NSBitmapImageRep(data: imgData),
                let pngImage = bitmap.representation(using: .png, properties: [:])
                else { return nil }
            
            return pngImage
        }
        return nil
    }
    
}
