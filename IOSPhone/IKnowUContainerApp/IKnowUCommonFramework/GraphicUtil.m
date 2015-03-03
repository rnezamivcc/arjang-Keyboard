//
//  GraphicUtil.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-08.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "GraphicUtil.h"

@implementation GraphicUtil

+ (UIImage *)imageWithImage:(UIImage *)image scaledToSize:(CGSize)newSize
{
    UIGraphicsBeginImageContextWithOptions(newSize, NO, 0.0);
    [image drawInRect:CGRectMake(0, 0, newSize.width, newSize.height)];
    UIImage *newImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return newImage;
}

+ (UIImage *)imageWithView:(UIView *)view
{
    UIGraphicsBeginImageContextWithOptions(view.bounds.size, view.opaque, 0.0f);
    [view.layer renderInContext:UIGraphicsGetCurrentContext()];
    UIImage *snapshotImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return snapshotImage;
}

// Overlay all non-alpha bits with a color
+ (UIImage *)overlayImage:(UIImage *)image withColor:(UIColor *)color
{
    // make a rectangle the size of your image
    CGRect rect = CGRectMake(0, 0, image.size.width, image.size.height);
    
    // create a new bitmap context based on the current image's size and scale, that has opacity
    UIGraphicsBeginImageContextWithOptions(rect.size, NO, image.scale);
    
    // get a reference to the current context (which you just created)
    CGContextRef c = UIGraphicsGetCurrentContext();
    
    // draw your image into the context we created
    [image drawInRect:rect];
    
    // set the fill color of the context
    CGContextSetFillColorWithColor(c, [color CGColor]);
    
    // this sets the blend mode.
    // basically it uses the your fill color with the alpha of the image and vice versa.
    CGContextSetBlendMode(c, kCGBlendModeSourceAtop);
    
    // now you apply the color and blend mode onto your context.
    CGContextFillRect(c, rect);
    
    // you grab the result of all this drawing from the context.
    UIImage *result = UIGraphicsGetImageFromCurrentImageContext();
    
    UIGraphicsEndImageContext();
    
    return result;
}

@end
