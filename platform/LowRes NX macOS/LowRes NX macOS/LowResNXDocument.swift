//
//  Document.swift
//  LowRes NX macOS
//
//  Created by Timo Kloss on 23/4/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import Cocoa

class ProgramError: NSError {
    
    init(errorCode: ErrorCode, lineNumber: Int, line: String) {
        let errorString = String(cString:err_getString(errorCode))
        let errorText = "Error in line \(lineNumber): \(errorString)\n\(line)"
        super.init(domain: "LowResNX", code: Int(errorCode.rawValue), userInfo: [
            NSLocalizedFailureReasonErrorKey: "There was a program error.",
            NSLocalizedRecoverySuggestionErrorKey: errorText
            ])
    }
    
    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
}

class LowResNXDocument: NSDocument {
    var sourceCode = ""
    var coreWrapper = CoreWrapper()
        
    override class func autosavesInPlace() -> Bool {
        return false
    }
    
    override func data(ofType typeName: String) throws -> Data {
        return sourceCode.data(using: .utf8)!
    }

    override func read(from data: Data, ofType typeName: String) throws {
        sourceCode = String(data: data, encoding: .ascii)!
        let cString = sourceCode.cString(using: .ascii)
        let errorCode = itp_compileProgram(&coreWrapper.core, cString)
        if errorCode != ErrorNone {
            throw getProgramError(errorCode: errorCode)
        }
    }
    
    func getProgramError(errorCode: ErrorCode) -> ProgramError {
        let cIndex = itp_getPcPositionInSourceCode(&coreWrapper.core)
        let index = sourceCode.index(sourceCode.startIndex, offsetBy: String.IndexDistance(cIndex))
        let lineRange = sourceCode.lineRange(for: index ..< index)
        let lineString = sourceCode.substring(with: lineRange)
        let lineNumber = sourceCode.countLines(index: index)
        return ProgramError(errorCode: errorCode, lineNumber: lineNumber, line: lineString)
    }
    
    override func makeWindowControllers() {
        let windowController = LowResNXWindowController(windowNibName: "LowResNXWindowController")
        addWindowController(windowController)
    }
    
    func nxDiskURL() -> URL {
        return fileURL!.deletingLastPathComponent().appendingPathComponent("disk.nx")
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
