//
//  IKUScrollView.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-14.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUScrollView.h"

@implementation IKUScrollView

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        self.delaysContentTouches = NO;
    }
    
    return self;
}

- (BOOL)touchesShouldCancelInContentView:(UIView *)view
{
    if ([view isKindOfClass:UIButton.class])
    {
        return YES;
    }
    
    return [super touchesShouldCancelInContentView:view];
}

@end
