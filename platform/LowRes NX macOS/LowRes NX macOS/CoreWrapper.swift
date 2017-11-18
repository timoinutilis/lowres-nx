//
//  CoreWrapper.swift
//  LowRes NX macOS
//
//  Created by Timo Kloss on 4/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import Cocoa

protocol CoreWrapperDelegate: class {
    func coreInterpreterDidFail(coreError: CoreError) -> Void
    func coreDiskDriveWillAccess(diskDataManager: UnsafeMutablePointer<DataManager>?) -> Bool
    func coreDiskDriveDidSave(diskDataManager: UnsafeMutablePointer<DataManager>?) -> Void
    func coreControlsDidChange(controlsInfo: ControlsInfo) -> Void
}

class CoreWrapper: NSObject {
    
    weak var delegate: CoreWrapperDelegate?
    
    var core = Core()
    private var coreDelegate = CoreDelegate()
    
    override init() {
        super.init()
        core_init(&core)
        
        coreDelegate.context = UnsafeMutableRawPointer(Unmanaged.passUnretained(self).toOpaque())
        coreDelegate.interpreterDidFail = interpreterDidFail
        coreDelegate.diskDriveWillAccess = diskDriveWillAccess
        coreDelegate.diskDriveDidSave = diskDriveDidSave
        coreDelegate.controlsDidChange = controlsDidChange
        core_setDelegate(&core, &coreDelegate)
    }
    
    deinit {
        core_deinit(&core)
    }
    
}

class LowResNXError: NSError {
    
    let coreError: CoreError
    
    init(error: CoreError, sourceCode: String) {
        coreError = error
        let index = sourceCode.index(sourceCode.startIndex, offsetBy: String.IndexDistance(error.sourcePosition))
        let lineRange = sourceCode.lineRange(for: index ..< index)
        let lineString = sourceCode[lineRange].trimmingCharacters(in: CharacterSet.whitespaces)
        let lineNumber = sourceCode.countLines(index: index)
        
        let errorString = String(cString:err_getString(error.code))
        let errorText = "Error in line \(lineNumber): \(errorString)\n\(lineString)"
        super.init(domain: "LowResNX", code: Int(error.code.rawValue), userInfo: [
            NSLocalizedFailureReasonErrorKey: "There was a program error.",
            NSLocalizedRecoverySuggestionErrorKey: errorText
            ])
    }
    
    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
}

extension String {

    func countLines(index: String.Index) -> Int {
        var count = 1
        var searchRange = startIndex ..< endIndex
        while let foundRange = rangeOfCharacter(from: CharacterSet.newlines, options: .literal, range: searchRange), index >= foundRange.upperBound {
            searchRange = characters.index(after: foundRange.lowerBound) ..< endIndex
            count += 1
        }
        return count;
    }
    
}

//MARK: - Core Delegate Functions Wrapper

func interpreterDidFail(context: UnsafeMutableRawPointer?, coreError: CoreError) -> Void {
    let wrapper = Unmanaged<CoreWrapper>.fromOpaque(context!).takeUnretainedValue()
    wrapper.delegate?.coreInterpreterDidFail(coreError: coreError)
}

func diskDriveWillAccess(context: UnsafeMutableRawPointer?, diskDataManager: UnsafeMutablePointer<DataManager>?) -> Bool {
    let wrapper = Unmanaged<CoreWrapper>.fromOpaque(context!).takeUnretainedValue()
    return wrapper.delegate?.coreDiskDriveWillAccess(diskDataManager: diskDataManager) ?? true
}

func diskDriveDidSave(context: UnsafeMutableRawPointer?, diskDataManager: UnsafeMutablePointer<DataManager>?) -> Void {
    let wrapper = Unmanaged<CoreWrapper>.fromOpaque(context!).takeUnretainedValue()
    wrapper.delegate?.coreDiskDriveDidSave(diskDataManager: diskDataManager)
}

func controlsDidChange(context: UnsafeMutableRawPointer?, controlsInfo: ControlsInfo) -> Void {
    let wrapper = Unmanaged<CoreWrapper>.fromOpaque(context!).takeUnretainedValue()
    wrapper.delegate?.coreControlsDidChange(controlsInfo: controlsInfo)
}
