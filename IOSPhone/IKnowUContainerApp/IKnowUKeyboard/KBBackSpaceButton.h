//
//  KBBackSpaceButton.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-21.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface KBBackSpaceButton : UIButton

@property (strong, nonatomic) UIImage *backSpaceImage;

- (void)initialize;
- (void)setFGColor:(UIColor *)color;

@end
