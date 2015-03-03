//
//  KBEmojiButton.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-21.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "GraphicUtil.h"
#import "KBEmojiButton.h"

@implementation KBEmojiButton

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
        // Call '[self initialize]' after setting self.frame
    }
    return self;
}

- (void)initialize
{    
    [self setFGColor:[UIColor whiteColor]];
    [self setContentMode:UIViewContentModeCenter];
    
    self.layer.cornerRadius = BUTTON_CORNER_RADIUS;
    self.clipsToBounds = YES;
}

- (void)setFGColor:(UIColor *)color
{
    CGFloat minHeight = MIN(self.frame.size.height, self.frame.size.width);
    CGFloat width = 2 * minHeight / 3;
    CGFloat height = 2 * minHeight / 3;
    self.emojiImage = [GraphicUtil imageWithImage:[UIImage imageNamed:@"key_smiley.png"]
                                     scaledToSize:CGSizeMake(width, height)];
    self.emojiImage = [GraphicUtil overlayImage:self.emojiImage withColor:color];
    
    [self setImage:self.emojiImage forState:UIControlStateNormal];
}

@end
