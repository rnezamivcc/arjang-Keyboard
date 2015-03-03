//
//  ExtendedPopupView.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-29.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "KBData.h"
#import "ExtendedPopupButton.h"
#import "ExtendedPopupView.h"

@implementation ExtendedPopupView

- (id)initWithKBData:(KBData *)kbdata popupStore:(ExtendedPopupStore *)store theme:(Theme *)theme
{
    if([kbdata.sender isKindOfClass:[KBKeyButton class]] && (store != nil))
    {
        self.currentTheme = theme;
        
        self.kbKeyButton = (KBKeyButton *)kbdata.sender;
        self.kbKeyButton.extendedPopupView = nil;
        CGRect frame = self.kbKeyButton.frame;
        
        NSString *aKey = (self.kbKeyButton.titleLabel.text != nil) ? self.kbKeyButton.titleLabel.text : @"";
        ExtendedPopupValues *values = [store getValues:aKey];
        NSArray *keyChars = [values getKeyChars];
        NSInteger size = [keyChars count];
        
        if (size <= 0)
        {
            return nil;
        }
        
        CGFloat buttonHeight = frame.size.height;
        CGFloat buttonWidth = POPUP_BUTTON_WIDTH;
        CGFloat popupHeight = 1.8 * buttonHeight + 4 * POPUP_BUTTON_SPACING;
        CGFloat popupWidth = POPUP_BUTTON_SPACING + (size * (buttonWidth + POPUP_BUTTON_SPACING));
        CGFloat buttonSmallFontSize = 10.0;
        CGFloat buttonFontSize = 15.0;
        self.buttonSpacing = POPUP_BUTTON_SPACING;
        
        if ([(NSString*)[UIDevice currentDevice].model hasPrefix:@"iPad"])
        {
            buttonHeight = frame.size.height;
            buttonWidth = POPUP_IPAD_BUTTON_WIDTH;
            popupHeight = 1.8 * buttonHeight + 4 * POPUP_IPAD_BUTTON_SPACING;
            popupWidth = POPUP_IPAD_BUTTON_SPACING + (size * (buttonWidth + POPUP_IPAD_BUTTON_SPACING));
            self.buttonSpacing = POPUP_IPAD_BUTTON_SPACING;
            buttonSmallFontSize = 15.0;
            buttonFontSize = 18.0;
        }
        
        NSInteger buttonWithInitFocus = 0;
        CGFloat x = MAX(1.0, frame.origin.x + (frame.size.width / 2) - (popupWidth / 2));
        if ((x + popupWidth) > (self.kbKeyButton.superview.frame.origin.x + self.kbKeyButton.superview.frame.size.width))
        {
            x = (self.kbKeyButton.superview.frame.origin.x + self.kbKeyButton.superview.frame.size.width) - popupWidth;
        }
        
        CGFloat y = frame.origin.y - (popupHeight - frame.size.height);
        CGRect popupFrame = CGRectMake(x, y, popupWidth, popupHeight - frame.size.height);
        
        self.buttonX = frame.origin.x - x;
        self.buttonWidth = frame.size.width;
        
        self = [self initWithFrame:popupFrame];
        if (self)
        {
            self.bkgdColor = (self.currentTheme != nil) ? UIColorFromRGB(self.currentTheme.previewBackgroundColor) : RGB(255, 255, 255);
            [self setBackgroundColor: [UIColor clearColor]];
            self.borderColor = [UIColor blackColor];
            self.kbKeyButton.extendedPopupView = self;
            
            self.layer.shadowOffset = CGSizeMake(0, 3);
            self.layer.shadowRadius = 6.0;
            self.layer.shadowColor = [[UIColor blackColor] CGColor];
            self.layer.shadowOpacity = 0.3;
            
            int offSet = 0;
            for (id object in keyChars)
            {
                ExtendedPopupButton *button = [[ExtendedPopupButton alloc] initWithFrame:CGRectZero];
                if ([object isKindOfClass:[NSString class]])
                {
                    NSString *s = (NSString *)object;
                    button.titleLabel.font = (s.length > 1) ? [UIFont systemFontOfSize:buttonSmallFontSize] :
                        [UIFont systemFontOfSize:buttonFontSize];
                }
                [button setTitle:object forState:UIControlStateNormal];
                
                if (offSet == buttonWithInitFocus)
                {
                    UIColor *bgColor = (self.currentTheme != nil) ? UIColorFromRGB(self.currentTheme.keyTextColor) : [UIColor whiteColor];
                    UIColor *fgColor = (self.currentTheme != nil) ? UIColorFromRGB(self.currentTheme.keyColor) : [UIColor darkGrayColor];
                    [self setButtonColors:button bg:bgColor fg:fgColor];
                }
                else
                {
                    UIColor *bgColor = (self.currentTheme != nil) ? UIColorFromRGB(self.currentTheme.keyColor) : [UIColor darkGrayColor];
                    UIColor *fgColor = (self.currentTheme != nil) ? UIColorFromRGB(self.currentTheme.keyTextColor) : [UIColor whiteColor];
                    [self setButtonColors:button bg:bgColor fg:fgColor];
                }
                button.frame = CGRectMake(self.buttonSpacing + offSet * (buttonWidth + self.buttonSpacing),
                                          self.buttonSpacing,
                                          buttonWidth, buttonHeight);
                [self addSubview:button];
                
                offSet++;
            }
        }
        return self;
    }
    else
    {
        return nil;
    }
}

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        // Initialization code
    }
    return self;
}

- (void)move:(CGPoint)location
{
     CGPoint point = [self.kbKeyButton convertPoint:location toView:self.superview];
    NSArray *subviews = self.subviews;
    
    int offSet = 0;
    for (id view in subviews)
    {
        CGRect selfFrame = self.frame;
        CGRect viewFrame = [view frame];
        
        CGRect rect = CGRectMake(selfFrame.origin.x + viewFrame.origin.x,
                                 selfFrame.origin.y + viewFrame.origin.y,
                                 viewFrame.size.width,
                                 2 * viewFrame.size.height + 2 * self.buttonSpacing);
        
        if (CGRectContainsPoint(rect, point))
        {
            UIColor *bgColor = (self.currentTheme != nil) ? UIColorFromRGB(self.currentTheme.keyTextColor) : [UIColor whiteColor];
            UIColor *fgColor = (self.currentTheme != nil) ? UIColorFromRGB(self.currentTheme.keyColor) : [UIColor darkGrayColor];
            [self setButtonColors:view bg:bgColor fg:fgColor];
        }
        else
        {
            UIColor *bgColor = (self.currentTheme != nil) ? UIColorFromRGB(self.currentTheme.keyColor) : [UIColor darkGrayColor];
            UIColor *fgColor = (self.currentTheme != nil) ? UIColorFromRGB(self.currentTheme.keyTextColor) : [UIColor whiteColor];
            [self setButtonColors:view bg:bgColor fg:fgColor];
        }
        offSet++;
    }
}

- (void)end:(CGPoint)location
{
    CGPoint point = [self.kbKeyButton convertPoint:location toView:self.superview];
    NSArray *subviews = self.subviews;
    
    int offSet = 0;
    for (id view in subviews)
    {
        CGRect selfFrame = self.frame;
        CGRect viewFrame = [view frame];
        CGRect rect = CGRectMake(selfFrame.origin.x + viewFrame.origin.x,
                                 selfFrame.origin.y + viewFrame.origin.y,
                                 viewFrame.size.width,
                                 2 * viewFrame.size.height + 2 * self.buttonSpacing);
        
        if (CGRectContainsPoint(rect, point))
        {
            UIButton *button = nil;
            if ([view isKindOfClass:[UIButton class]])
            {
                button = (UIButton *)view;
            }
            if (button != nil)
            {
                KBData *data = [[KBData alloc] init];
                NSString *dataString = (button.titleLabel.text != nil) ? button.titleLabel.text : @"";
                [data.s setString:dataString];
                data.tag = button.tag;
                [self.eventsHandler handleKeyboardData:data];
            }
        }
        offSet++;
    }
}

// Set background/foreground colour of button. Use to highlight/unhighlight button
- (void)setButtonColors:(UIView *)view bg:(UIColor *)bgColor fg:(UIColor *)fgColor
{
    UIButton *button = nil;
    if ([view isKindOfClass:[UIButton class]])
    {
        button = (UIButton *)view;
    }
    if (button != nil)
    {
        button.backgroundColor = bgColor;
        [button setTitleColor:fgColor forState:UIControlStateNormal];
    }
}

- (void)drawRect:(CGRect)rect
{
    CGContextRef c = UIGraphicsGetCurrentContext();
    
    CGContextSetRGBStrokeColor(c, 0.0, 0.0, 0.0, 1.0);
    CGContextSetLineWidth(c, 1.0);
    
    CGMutablePathRef bubblePath = CGPathCreateMutable();
    
    CGFloat cornerRadius = 6.0;
    CGFloat pointerSize = 6.0;
    CGPathMoveToPoint(bubblePath, NULL, self.buttonX + self.buttonWidth / 2,
                                        rect.origin.y + rect.size.height);
    CGPathAddLineToPoint(bubblePath, NULL, self.buttonX + self.buttonWidth / 2 - pointerSize,
                                           rect.origin.y + rect.size.height - pointerSize);
    
    CGPathAddArcToPoint(bubblePath, NULL,
                        rect.origin.x, rect.origin.y + rect.size.height - pointerSize,
                        rect.origin.x, rect.origin.y + rect.size.height - pointerSize - cornerRadius,
                        cornerRadius);
    
    CGPathAddArcToPoint(bubblePath, NULL,
                        rect.origin.x, rect.origin.y,
                        rect.origin.x + cornerRadius, rect.origin.y,
                        cornerRadius);
    
    CGPathAddArcToPoint(bubblePath, NULL,
                        rect.origin.x + rect.size.width, rect.origin.y,
                        rect.origin.x + rect.size.width, rect.origin.y + cornerRadius,
                        cornerRadius);
    
    CGPathAddArcToPoint(bubblePath, NULL,
                        rect.origin.x + rect.size.width, rect.origin.y + rect.size.height - pointerSize,
                        rect.origin.x + rect.size.width - cornerRadius, rect.origin.y + rect.size.height - pointerSize,
                        cornerRadius);
    
    CGPathAddLineToPoint(bubblePath, NULL, self.buttonX + self.buttonWidth / 2 + pointerSize,
                                           rect.origin.y + rect.size.height - pointerSize);
    
    CGPathCloseSubpath(bubblePath);
    
    CGContextSaveGState(c);
    CGContextAddPath(c, bubblePath);
    CGContextClip(c);
    
    CGContextSetFillColorWithColor(c, [self.bkgdColor CGColor]);
    CGContextFillRect(c, self.bounds);
    
    CGContextRestoreGState(c);
    
    size_t numBorderComponents = CGColorGetNumberOfComponents([self.borderColor CGColor]);
    const CGFloat *borderComponents = CGColorGetComponents(self.borderColor.CGColor);
    CGFloat r, g, b, a;
    if (numBorderComponents == 2)
    {
        r = borderComponents[0];
        g = borderComponents[0];
        b = borderComponents[0];
        a = borderComponents[1];
    }
    else
    {
        r = borderComponents[0];
        g = borderComponents[1];
        b = borderComponents[2];
        a = borderComponents[3];
    }
    
    CGContextSetRGBStrokeColor(c, r, g, b, a);
    CGContextAddPath(c, bubblePath);
    CGContextDrawPath(c, kCGPathStroke);
    
    CGPathRelease(bubblePath);
}

@end
