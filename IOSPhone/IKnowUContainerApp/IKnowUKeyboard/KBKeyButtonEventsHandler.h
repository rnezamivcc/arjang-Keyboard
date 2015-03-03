//
//  KBKeyButtonEventsHandler.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-01.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol KBKeyButtonEventsHandler

- (void)handleKBKeyButtonDown:(id)sender withEvent:(UIEvent *)event location:(CGPoint)location;
- (void)handleKBKeyButtonMove:(id)sender withEvent:(UIEvent *)event location:(CGPoint)location;
- (void)handleKBKeyButtonUp:(id)sender withEvent:(UIEvent *)event location:(CGPoint)location;

@end