//
//  SwipeView.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-31.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface SwipeView : UIView

@property (nonatomic) Boolean isEnabled;
@property (nonatomic) Boolean isDrawing;
@property (nonatomic, strong) NSMutableArray *locations;
@property (nonatomic, strong) NSMutableArray *discardedItems;
@property (weak) NSTimer *updateLocationsTimer;

- (void)startLocation:(CGPoint)location;
- (void)addLocation:(CGPoint)location;
- (void)endLocation:(CGPoint)location;

@end
