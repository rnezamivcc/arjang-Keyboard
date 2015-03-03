//
//  KBData.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-17.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "KBData.h"

@implementation KBData

- (id)init
{
    self = [super init];
    if (self)
    {
        self.s = [[NSMutableString alloc] init];
    }
    return self;
}

- (void)reset
{
    self.isPhrase = false;
    self.isPrediction = false;
    self.isLongPressedStart = false;
    self.isLongPressedEnd = false;
    self.isShifted = false;
    self.isShiftLocked = false;
    self.isEmoji = false;
    self.isBackSpace = false;
    self.tag = -1;
    self.popupState = kDoNothing;
    self.swipeAction = kSwipeNone;
    [self.s setString:@""];
    self.sender = nil;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"obj = %@ s[%@] sender[%@] tag[%ld] "
                                        "isPhrase[%d] isPrediction[%d] isLongPressedStart[%d] isLongPressedEnd[%d] isShifted[%d] isShiftLocked[%d] isEmoji[%d] "
                                        "isBackSpace[%d] popupState[%lu] swipeAction[%lu]",
            [super description], self.s, self.sender, (long)self.tag,
            self.isPhrase, self.isPrediction, self.isLongPressedStart, self.isLongPressedEnd, self.isShifted, self.isShiftLocked, self.isEmoji,
            self.isBackSpace, (unsigned long)self.popupState, (unsigned long)self.swipeAction];
}

@end
