//
// Copyright 2016 Timo Kloss
//
// This file is part of LowRes Core.
//
// LowRes Core is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LowRes Core is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with LowRes Core.  If not, see <http://www.gnu.org/licenses/>.
//

#import "ViewController.h"
#import "interpreter.h"

@interface ViewController ()
@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    NSString *filePath = [[NSBundle mainBundle] pathForResource:@"demo" ofType:@"bas" inDirectory:@"bas"];
    NSString *demoProgram = [NSString stringWithContentsOfFile:filePath encoding:NSASCIIStringEncoding error:nil];
    
    Interpreter interpreter;
    ErrorCode errorCode = LRC_tokenizeProgram(&interpreter, [demoProgram cStringUsingEncoding:NSASCIIStringEncoding]);
    if (errorCode != ErrorNone)
    {
        printf("Tokenizer error: %d\n", errorCode);
    }
    else
    {
        printf("Tokenizer success\n");
        // run!
    }
}

- (BOOL)prefersStatusBarHidden
{
    return YES;
}

@end
