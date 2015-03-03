//
//  KeyboardLayoutFactory.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-15.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

@interface KeyboardLayoutFactory : NSObject

+ (UIView *)createLettersView:(CGFloat)width withHeight:(CGFloat)height;
+ (UIView *)createNumbersView:(CGFloat)width withHeight:(CGFloat)height;
+ (UIView *)createSymbolsView:(CGFloat)width withHeight:(CGFloat)height;
+ (UIView *)createEmojiView:(CGFloat)width withHeight:(CGFloat)height;
+ (UIView *)createSuggestionsView:(CGFloat)width withHeight:(CGFloat)height;

@end
