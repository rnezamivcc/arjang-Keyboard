//
//  MAData.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-13.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MAData : NSObject

typedef NS_ENUM(NSUInteger, MiniAppCategory)
{
    kNoMiniAppCategory,
    kContactsMiniAppCategory,
    kLocationsMiniAppCategory
};

typedef NS_ENUM(NSUInteger, MiniAppAction)
{
    kNoMiniAppAction,
    kDoneMiniAppAction,
    kClipMiniAppAction
};

@property (strong, nonatomic) NSMutableString *s;
@property (nonatomic) MiniAppCategory miniAppCategory;
@property (nonatomic) MiniAppAction miniAppAction;

- (void)reset;

@end
