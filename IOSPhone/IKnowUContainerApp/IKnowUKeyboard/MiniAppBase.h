//
//  MiniAppBase.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-08.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MiniAppBarIcon.h"

@interface MiniAppBase : NSObject

@property (nonatomic, strong) MiniAppBarIcon *smallIcon;
@property (nonatomic, strong) MiniAppBarIcon *bigIcon;
@property (nonatomic, strong) NSMutableString *search;
@property (nonatomic, strong) UIView *parentView;

- (void)setSearchParams:(NSString *)searchParam;
- (void)engage:(UIView *)view;
- (void)clearParentView;
- (void)resetContentDimensions:(CGRect)rect;

@end
