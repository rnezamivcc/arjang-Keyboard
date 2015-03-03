//
//  ExtendedPopupValues.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-30.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "ExtendedPopupValues.h"

@implementation ExtendedPopupValues

- (id)initWithChr:(NSString *)chr
{
    self = [super init];
    if (self)
    {
        self.theChar = [[NSMutableString alloc] init];
        [self.theChar setString:chr];
        self.defaultChar = [[NSMutableString alloc] init];
        self.keyChars = [[NSMutableArray alloc] init];
        return self;
    }
    return nil;
}

- (void)addKeyChar:(NSString *)chr isDefault:(NSString *)isDefault
{
    [self.keyChars addObject:chr];
    if ((isDefault != nil) && [isDefault isEqualToString:@"true"])
    {
        [self.defaultChar setString:chr];
    }
}

- (NSArray *)getKeyChars
{
    return self.keyChars;
}

- (NSString *)getMainChar
{
    return self.theChar;
}

- (NSString *)getDefaultChar
{
    return self.defaultChar;
}

- (NSString *)description
{
    NSMutableString *buffer = [[NSMutableString alloc] init];
    
    [buffer appendFormat:@"theChar[%@] ", self.theChar];
    [buffer appendString:@"keyChars[ "];
    for (id object in self.keyChars)
    {
        [buffer appendFormat:@"%@ ", object];
    }
    [buffer appendString:@"]"];
    
    [buffer appendFormat:@"defaultChar[%@]", self.defaultChar];
    
    return buffer;
}

@end
