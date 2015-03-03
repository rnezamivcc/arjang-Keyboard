//
//  ExtendedPopupView.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-29.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "KBData.h"
#import "ExtendedPopupStore.h"
#import "KBKeyButton.h"
#import "KeyboardViewEventsHandler.h"
#import "KeyboardView.h"
#import "Theme.h"

@class KBKeyButton;
@interface ExtendedPopupView : UIView

@property (weak) id <KeyboardViewEventsHandler> eventsHandler;
@property (strong, nonatomic) KBKeyButton *kbKeyButton;
@property (strong, nonatomic) ExtendedPopupStore *popupStore;
@property (strong, nonatomic) UIColor *bkgdColor;
@property (strong, nonatomic) UIColor *borderColor;
@property (strong, nonatomic) Theme *currentTheme;
@property (nonatomic) CGFloat buttonX;
@property (nonatomic) CGFloat buttonWidth;
@property (nonatomic) CGFloat buttonSpacing;

- (id)initWithKBData:(KBData *)kbdata popupStore:(ExtendedPopupStore *)store theme:(Theme *)theme;
- (void)move:(CGPoint)location;
- (void)end:(CGPoint)location;

@end
