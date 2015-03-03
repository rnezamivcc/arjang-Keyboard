//
//  LayoutManager.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-18.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

@interface LayoutManager : NSObject

typedef NS_ENUM(NSUInteger, KBLayoutType)
{
    kKBLettersType,
    kKBNumbersType,
    kKBSymbolsType,
    kKBEmojiType,
    kKBSuggestionsType
};

typedef NS_ENUM(NSUInteger, KBLayoutState)
{
    kKBPortraitState,
    kKBLandscapeState
};

@property (nonatomic, strong) NSMutableDictionary *layouts;
@property (nonatomic) KBLayoutType currentLayoutType;
@property (nonatomic) KBLayoutState layoutState;

- (void)addKBLayout:(UIView *)layout byType:(KBLayoutType)type byState:(KBLayoutState)state;
- (id)getKBLayout:(KBLayoutType)type;

@end
