//
//  SwipeView.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-31.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "SwipeLocation.h"
#import "SwipeView.h"

#define INITIAL_CAPACITY 50

@implementation SwipeView

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        self.backgroundColor = [UIColor clearColor];
        self.locations = [[NSMutableArray alloc] initWithCapacity:INITIAL_CAPACITY];
        self.discardedItems = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)startLocation:(CGPoint)location
{
    [self.locations removeAllObjects];
    [self.locations addObject:[[SwipeLocation alloc] initWithLocation:location andAlpha:1.0]];
    self.isDrawing = true;
    [self setNeedsDisplay];
    
    self.updateLocationsTimer = [NSTimer scheduledTimerWithTimeInterval:0.05f
                                                                 target:self
                                                               selector:@selector(updateLocations:)
                                                               userInfo:nil
                                                                repeats:YES];
}

- (void)updateLocations:(NSTimer *)timer
{
    [self.discardedItems removeAllObjects];
    for (SwipeLocation *item in self.locations)
    {
        item.alpha -= .05;
        if (item.alpha <= 0.0)
        {
            [self.discardedItems addObject:item];
        }
    }
    [self.locations removeObjectsInArray:self.discardedItems];
    [self setNeedsDisplay];
}

- (void)stopUpdateLocationsTimer
{
    if ([self.updateLocationsTimer isValid])
    {
        [self.updateLocationsTimer invalidate];
    }
    self.updateLocationsTimer = nil;
}

- (void)addLocation:(CGPoint)location
{
    NSInteger size = [self.locations count];
    if (size >= INITIAL_CAPACITY)
    {
        [self.locations removeObjectAtIndex:0];
    }
    [self.locations addObject:[[SwipeLocation alloc] initWithLocation:location andAlpha:1.0]];
    [self setNeedsDisplay];
}

- (void)endLocation:(CGPoint)location
{
    [self.locations removeAllObjects];
    [self.discardedItems removeAllObjects];
    [self stopUpdateLocationsTimer];
    self.isDrawing = false;
    [self setNeedsDisplay];
}

- (void)drawRect:(CGRect)rect
{
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    CGContextSetStrokeColorWithColor(context, [UIColor blueColor].CGColor);
    CGContextSetLineWidth(context, 5.0f);
    int i = 0;
    for (SwipeLocation *item in self.locations)
    {
        if (i == 0)
        {
            CGContextSetAlpha(context, item.alpha);
            CGContextMoveToPoint(context, item.location.x, item.location.y);
        }
        else
        {
            CGContextSetAlpha(context, item.alpha);
            CGContextAddLineToPoint(context, item.location.x, item.location.y);
        }
        i++;
    }
    CGContextStrokePath(context);
}

@end
