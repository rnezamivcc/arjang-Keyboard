//
//  KBKeyButton.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-28.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "KBKeyButton.h"

@implementation KBKeyButton

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

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesBegan:touches withEvent:event];
    
    UITouch *aTouch = [touches anyObject];
    CGPoint location = [aTouch locationInView:self];
    
    [self.keyButtonEventsHandler handleKBKeyButtonDown:self withEvent:event location:location];
    
    [self.nextResponder touchesBegan:touches withEvent:event];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesMoved:touches withEvent:event];
    
    UITouch *aTouch = [touches anyObject];
    CGPoint location = [aTouch locationInView:self];
    
    if (self.extendedPopupView)
    {
        [self.extendedPopupView move:location];
    }
    
    [self.keyButtonEventsHandler handleKBKeyButtonMove:self withEvent:event location:location];
    
    [self.nextResponder touchesMoved:touches withEvent:event];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesEnded:touches withEvent:event];
    
    UITouch *aTouch = [touches anyObject];
    CGPoint location = [aTouch locationInView:self];
    
    if (self.extendedPopupView)
    {
        [self.extendedPopupView end:location];
    }
    
    [self.keyButtonEventsHandler handleKBKeyButtonUp:self withEvent:event location:location];
    
    [self.nextResponder touchesEnded:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesCancelled:touches withEvent:event];
    
    [self.nextResponder touchesCancelled:touches withEvent:event];
}

@end
