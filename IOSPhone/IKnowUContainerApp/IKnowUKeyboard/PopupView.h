//
//  PopupView.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-24.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "KBData.h"
#import "Theme.h"

@interface PopupView : UIView

@property (strong, nonatomic) NSMutableString *s;
@property (strong, nonatomic) UIColor *bkgdColor;
@property (strong, nonatomic) UIColor *borderColor;
@property (strong, nonatomic) UIFont *font;
@property (strong, nonatomic) Theme *currentTheme;

- (instancetype)initWithKBData:(KBData *)kbdata theme:(Theme *)theme;

@end
