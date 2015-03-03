//
//  KBSymbolsView.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-11.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "KBSymbolsView.h"

@implementation KBSymbolsView

- (id)initWithCoder:(NSCoder*)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self)
    {
        [self initialize];
    }
    return self;
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        // Call '[self initialize]' after adding all subviews
    }
    return self;
}

- (void)initialize
{
    [super initialize];
    
    NSArray *subviews = self.subviews;
    for (id view in subviews)
    {
        if([view isKindOfClass:[KBKeyButton class]])
        {
            KBKeyButton *button = (KBKeyButton *)view;
            button.keyButtonEventsHandler = self;
        }
    }
}

- (void)updateWithTheme:(Theme *)theme
{
    [super updateWithTheme:theme];
    if (theme)
    {
        NSArray *subviews = self.subviews;
        for (id view in subviews)
        {
            if ([view isKindOfClass:[KBNextInputModeButton class]])
            {
                KBNextInputModeButton *button = (KBNextInputModeButton *)view;
                [button setFGColor:UIColorFromRGB(theme.keyTextColor)];
            }
            
            else if ([view isKindOfClass:[KBEmojiButton class]])
            {
                KBEmojiButton *button = (KBEmojiButton *)view;
                [button setFGColor:UIColorFromRGB(theme.keyTextColor)];
            }
            
            else if ([view isKindOfClass:[KBBackSpaceButton class]])
            {
                KBBackSpaceButton *button = (KBBackSpaceButton *)view;
                [button setFGColor:UIColorFromRGB(theme.keyTextColor)];
            }
        }
    }
}

- (IBAction)buttonPressed:(id)sender
{
    // Close the popup
    [self.data reset];
    self.data.popupState = kClosePopup;
    [self handlePopup:self.data];
    
    // Send the data
    [self handlePress:sender];
}

- (IBAction)buttonLongPressed:(id)sender
{
    UILongPressGestureRecognizer *gesture = (UILongPressGestureRecognizer *)sender;
    UIButton *button = (UIButton *)gesture.view;
    CGPoint location = [gesture locationInView:button];
    
    [self.data reset];
    switch (gesture.state)
    {
        case UIGestureRecognizerStateEnded:
        {
            // Close the popup
            [self.data reset];
            self.data.popupState = kClosePopup;
            [self handlePopup:self.data];
            break;
        }
            
        case UIGestureRecognizerStateChanged:
        {
            if ([button pointInside:location withEvent:nil])
            {
                // inside
            }
            else
            {
                // outside
                
                // Close the popup
                [self.data reset];
                self.data.popupState = kClosePopup;
                [self handlePopup:self.data];
            }
            break;
        }
            
        case UIGestureRecognizerStateBegan:
        case UIGestureRecognizerStateCancelled:
        case UIGestureRecognizerStateFailed:
        case UIGestureRecognizerStatePossible:
            break;
    }

    [self handleLongPress:sender];
}

@end
