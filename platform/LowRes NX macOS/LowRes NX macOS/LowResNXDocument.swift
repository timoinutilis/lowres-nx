//
//  Document.swift
//  LowRes NX macOS
//
//  Created by Timo Kloss on 23/4/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import Cocoa

class LowResNXDocument: NSDocument {
    var sourceCode = ""
    var coreWrapper = CoreWrapper()
        
    override class var autosavesInPlace: Bool {
        return false
    }
    
    override func data(ofType typeName: String) throws -> Data {
        return sourceCode.data(using: .utf8)!
    }

    override func read(from data: Data, ofType typeName: String) throws {
        sourceCode = String(data: data, encoding: .ascii)!
        let cString = sourceCode.cString(using: .utf8)
        let error = itp_compileProgram(&coreWrapper.core, cString)
        if error.code != ErrorNone {
            throw LowResNXError(error: error, sourceCode: sourceCode)
        }
    }
    
    override func makeWindowControllers() {
        let windowController = LowResNXWindowController(windowNibName: NSNib.Name(rawValue: "LowResNXWindowController"))
        addWindowController(windowController)
    }
    
}

