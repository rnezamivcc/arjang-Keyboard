//
//  MiniAppBarView.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-08.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "MiniAppBarView.h"

@implementation MiniAppBarView

- (void)clearAllMiniApps
{
    NSArray *subviews = self.subviews;
    for (id view in subviews)
    {
        [view removeFromSuperview];
    }
}

- (void)addMiniApp:(MiniAppBase *)miniApp
{
    NSArray *subviews = self.subviews;
    miniApp.smallIcon.frame = CGRectMake(0.0 + ([subviews count] * (MINI_APP_BAR_HEIGHT + 3)), 0.0,
                                         MINI_APP_BAR_HEIGHT, MINI_APP_BAR_HEIGHT);
    
    [self addSubview:miniApp.smallIcon];
}

@end
