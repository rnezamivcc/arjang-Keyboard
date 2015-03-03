//
//  SwipeLocation.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-25.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

@interface SwipeLocation : NSObject

@property (nonatomic) CGPoint location;
@property (nonatomic) CGFloat alpha;

- (id)initWithLocation:(CGPoint)l andAlpha:(CGFloat)a;

@end
