//
//  ExtendedPopupButton.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-26.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "ExtendedPopupButton.h"

@implementation ExtendedPopupButton

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        self.layer.cornerRadius = BUTTON_CORNER_RADIUS;
        self.clipsToBounds = YES;
    }
    return self;
}

@end
