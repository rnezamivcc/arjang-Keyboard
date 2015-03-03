//
//  GraphicUtil.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-08.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface GraphicUtil : NSObject

+ (UIImage *)imageWithImage:(UIImage *)image scaledToSize:(CGSize)newSize;
+ (UIImage *)imageWithView:(UIView *)view;
+ (UIImage *)overlayImage:(UIImage *)image withColor:(UIColor *)color;

@end
