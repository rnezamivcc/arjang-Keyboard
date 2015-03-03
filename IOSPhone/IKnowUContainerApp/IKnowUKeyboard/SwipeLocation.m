//
//  SwipeLocation.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-25.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "SwipeLocation.h"

@implementation SwipeLocation

- (id)initWithLocation:(CGPoint)l andAlpha:(CGFloat)a
{
    self = [super init];
    if (self)
    {
        self.location = l;
        self.alpha = a;
    }
    return self;
}

@end
