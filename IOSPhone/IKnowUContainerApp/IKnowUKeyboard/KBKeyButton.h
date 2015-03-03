//
//  KBKeyButton.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-28.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "KBKeyButtonEventsHandler.h"
#import "ExtendedPopupView.h"

@class ExtendedPopupView;
@interface KBKeyButton : UIButton

@property (weak) id <KBKeyButtonEventsHandler> keyButtonEventsHandler;
@property (weak) ExtendedPopupView *extendedPopupView;

@end
