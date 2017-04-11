//
//  NSString+Utils.m
//  Pixels
//
//  Created by Timo Kloss on 18/1/15.
//  Copyright (c) 2015 Inutilis Software. All rights reserved.
//

#import "NSString+Utils.h"

@implementation NSString (Utils)

+ (NSString *)stringWithChar:(char)character count:(NSInteger)count
{
    NSMutableString *string = [NSMutableString stringWithCapacity:count];
    for (NSInteger i = 0; i < count; i++)
    {
        [string appendFormat:@"%c", character];
    }
    return string;
}

- (NSUInteger)countLines
{
    return [self countLinesUntilIndex:self.length];
}

- (NSUInteger)countLinesUntilIndex:(NSUInteger)endIndex
{
    NSUInteger numberOfLines, index;
    for (index = 0, numberOfLines = 0; index < endIndex; numberOfLines++)
    {
        index = NSMaxRange([self lineRangeForRange:NSMakeRange(index, 0)]);
    }
    return numberOfLines;
}

- (NSUInteger)countChar:(unichar)character
{
    NSUInteger number = 0;
    for (NSUInteger pos = 0; pos < self.length; pos++)
    {
        if ([self characterAtIndex:pos] == character)
        {
            number++;
        }
    }
    return number;
}

- (NSString *)stringWithMaxWords:(int)maxWords
{
    NSArray *parts = [self componentsSeparatedByString:@" "];
    if (parts.count > maxWords)
    {
        NSMutableArray *mutableParts = parts.mutableCopy;
        [mutableParts removeObjectsInRange:NSMakeRange(maxWords, parts.count - maxWords)];
        NSString *shortString = [mutableParts componentsJoinedByString:@" "];
        return [NSString stringWithFormat:@"%@…", shortString];
    }
    return self;
}

@end
