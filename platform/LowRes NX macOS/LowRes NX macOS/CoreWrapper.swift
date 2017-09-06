//
//  CoreWrapper.swift
//  LowRes NX macOS
//
//  Created by Timo Kloss on 4/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import Cocoa

class CoreWrapper: NSObject {
    
    var core = Core()
    
    override init() {
        super.init()
        core_init(&core)
    }
    
    deinit {
        core_deinit(&core)
    }
    
}
