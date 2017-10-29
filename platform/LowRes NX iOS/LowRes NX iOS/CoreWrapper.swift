//
//  CoreWrapper.swift
//  LowRes NX iOS
//
//  Created by Timo Kloss on 2/9/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

import UIKit

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
