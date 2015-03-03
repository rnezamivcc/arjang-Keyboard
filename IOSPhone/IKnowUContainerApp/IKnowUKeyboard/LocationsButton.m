//
//  LocationsButton.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-03.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "LocationsButton.h"

@implementation LocationsButton

- (id)initWithCoder:(NSCoder*)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self)
    {
        self.layer.cornerRadius = BUTTON_CORNER_RADIUS;
        self.clipsToBounds = YES;
    }
    return self;
}

@end
