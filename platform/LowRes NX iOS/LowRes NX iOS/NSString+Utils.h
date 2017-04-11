//
//  NSString+Utils.h
//  Pixels
//
//  Created by Timo Kloss on 18/1/15.
//  Copyright (c) 2015 Inutilis Software. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSString (Utils)

+ (NSString *)stringWithChar:(char)character count:(NSInteger)count;

- (NSUInteger)countLines;
- (NSUInteger)countLinesUntilIndex:(NSUInteger)endIndex;
- (NSUInteger)countChar:(unichar)character;
- (NSString *)stringWithMaxWords:(int)maxWords;

@end
