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
        super.init(domain: "LowResNX", code: Int(errorCode.rawValue), userInfo: [NSLocalizedRecoverySuggestionErrorKey: errorText])
    }
    
    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
}

class Document: NSDocument {
    var sourceCode = ""
    var core = Core()
    var timer: Timer? = nil
    
    @IBOutlet weak var lowResNXView: LowResNXView!
    
    override init() {
        super.init()
        core_init(&core)
    }
    
    override class func autosavesInPlace() -> Bool {
        return false
    }

    override var windowNibName: String? {
        return "Document"
    }

    override func data(ofType typeName: String) throws -> Data {
        return sourceCode.data(using: .utf8)!
    }

    override func read(from data: Data, ofType typeName: String) throws {
        sourceCode = String(data: data, encoding: .utf8)!
        let errorCode = itp_compileProgram(&core, sourceCode.cString(using: .utf8))
        if errorCode != ErrorNone {
            throw ProgramError(errorCode: errorCode, lineNumber: 0, line: "TODO")
        } else {
            
        }
    }
    
    override func windowControllerDidLoadNib(_ windowController: NSWindowController) {
        super.windowControllerDidLoadNib(windowController);

        lowResNXView.core = UnsafeMutablePointer<Core>(&core)
        timer = Timer.scheduledTimer(timeInterval: 1.0/30.0, target: self, selector: #selector(Document.update), userInfo: nil, repeats: true)
    }
    
    func update() {
        core_update(&core)
        lowResNXView?.render()
    }
    
}

