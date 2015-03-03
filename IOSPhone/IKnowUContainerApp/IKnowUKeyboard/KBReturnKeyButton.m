//
//  KBReturnKeyButton.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-25.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "KBReturnKeyButton.h"

@implementation KBReturnKeyButton

- (id)initWithCoder:(NSCoder*)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self)
    {
        [self initialize];
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        [self initialize];
    }
    return self;
}

- (void)initialize
{
    self.layer.cornerRadius = BUTTON_CORNER_RADIUS;
    self.clipsToBounds = YES;
}

@end
