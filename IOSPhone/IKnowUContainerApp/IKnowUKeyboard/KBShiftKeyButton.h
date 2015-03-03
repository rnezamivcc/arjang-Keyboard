//
//  KBShiftKeyButton.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-18.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "KBShiftKeyButtonEventsHandler.h"

@interface KBShiftKeyButton : UIButton

typedef NS_ENUM(NSUInteger, ShiftKeyState)
{
    kShiftKeyLower,
    kShiftKeyUpper,
    kShiftKeyUpperLocked
};

@property (weak) id <KBShiftKeyButtonEventsHandler> shiftKeyButtonEventsHandler;

@property (nonatomic) ShiftKeyState skState;
@property (strong, nonatomic) UIImage *shiftImage;
@property (strong, nonatomic) UIImage *shiftUpImage;
@property (strong, nonatomic) UIImage *shiftLockImage;

- (void)initialize;
- (void)setFGColor:(UIColor *)color;
- (void)updateAppearance;

@end
