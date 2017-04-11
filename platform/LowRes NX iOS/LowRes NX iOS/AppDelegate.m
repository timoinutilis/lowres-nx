//
// Copyright 2016 Timo Kloss
//
// This file is part of LowRes NX.
//
// LowRes NX is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LowRes NX is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with LowRes NX.  If not, see <http://www.gnu.org/licenses/>.
//

#import "AppDelegate.h"
#import "core.h"
#import "NSString+Utils.h"
#import "ViewController.h"

@interface AppDelegate ()
@property (nonatomic) NSString *programSourceCode;
@end

@implementation AppDelegate {
    struct Core *_core;
}

- (void)dealloc
{
    if (_core)
    {
        itp_freeProgram(_core);
        free(_core);
    }
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // Override point for customization after application launch.
    
    _core = calloc(1, sizeof(struct Core));
    if (!_core)
    {
        NSLog(@"Alloc core failed");
    }
    else
    {
        core_init(_core);
        
        NSString *filePath = [[NSBundle mainBundle] pathForResource:@"menu" ofType:@"bas" inDirectory:@"bas"];
        self.programSourceCode = [NSString stringWithContentsOfFile:filePath encoding:NSASCIIStringEncoding error:nil];
        
        enum ErrorCode errorCode = itp_compileProgram(_core, [self.programSourceCode cStringUsingEncoding:NSASCIIStringEncoding]);
        if (errorCode != ErrorNone)
        {
            [self showErrorWithCode:errorCode];
        }
        else
        {
            NSLog(@"Compiler success");
        }
    }
    
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}


- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


- (struct Core*)getCore
{
    return _core;
}

- (void)showErrorWithCode:(enum ErrorCode)errorCode
{
    int pos = _core->interpreter.pc->sourcePosition;
    NSUInteger lineStart, lineEnd;
    [self.programSourceCode getLineStart:&lineStart end:&lineEnd contentsEnd:NULL forRange:NSMakeRange(pos, 0)];
    NSString *lineString = [self.programSourceCode substringWithRange:NSMakeRange(lineStart, lineEnd - lineStart)];
    NSUInteger lineNumber = [self.programSourceCode countLinesUntilIndex:pos];
    NSLog(@"Error at position %d (line %lu): %s\n", pos, (unsigned long)lineNumber, ErrorStrings[errorCode]);
    NSLog(@"%@", lineString);
    NSLog(@"%@^", [NSString stringWithChar:' ' count:(pos - lineStart)]);
}

@end
