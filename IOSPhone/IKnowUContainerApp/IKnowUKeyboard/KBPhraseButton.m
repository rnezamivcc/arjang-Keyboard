//
//  KBPhraseButton.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-21.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "KBPhraseButton.h"

@implementation KBPhraseButton

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
    self.s = [[NSMutableString alloc] init];
    self.titleLabel.textAlignment = NSTextAlignmentCenter;
    self.layer.cornerRadius = BUTTON_CORNER_RADIUS;
    self.clipsToBounds = YES;
}

@end
