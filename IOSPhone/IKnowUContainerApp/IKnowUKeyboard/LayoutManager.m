//
//  LayoutManager.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-18.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "LayoutManager.h"

@implementation LayoutManager

- (id)init
{
    self = [super init];
    if (self)
    {
        self.layouts = [[NSMutableDictionary alloc] init];
        self.layoutState = kKBPortraitState;
        self.currentLayoutType = kKBLettersType;
    }
    return self;
}

- (void)addKBLayout:(UIView *)layout byType:(KBLayoutType)type byState:(KBLayoutState)state
{
    if (state == kKBLandscapeState) [self addKBLandscapeLayout:layout byType:type];
    if (state == kKBPortraitState)  [self addKBPortraitLayout:layout byType:type];
}

- (id)getKBLayout:(KBLayoutType)type
{
    if (self.layoutState == kKBLandscapeState) return [self getKBLandscapeLayout:type];
    if (self.layoutState == kKBPortraitState)  return [self getKBPortraitLayout:type];
    return nil;
}

- (void)addKBPortraitLayout:(UIView *)layout byType:(KBLayoutType)type
{
    if (type == kKBLettersType)     self.layouts[@"letters"] = layout;
    if (type == kKBNumbersType)     self.layouts[@"numbers"] = layout;
    if (type == kKBSymbolsType)     self.layouts[@"symbols"] = layout;
    if (type == kKBSuggestionsType) self.layouts[@"suggestions"] = layout;
    if (type == kKBEmojiType)       self.layouts[@"emoji"] = layout;
}

- (void)addKBLandscapeLayout:(UIView *)layout byType:(KBLayoutType)type
{
    if (type == kKBLettersType)     self.layouts[@"letters_landscape"] = layout;
    if (type == kKBNumbersType)     self.layouts[@"numbers_landscape"] = layout;
    if (type == kKBSymbolsType)     self.layouts[@"symbols_landscape"] = layout;
    if (type == kKBSuggestionsType) self.layouts[@"suggestions_landscape"] = layout;
    if (type == kKBEmojiType)       self.layouts[@"emoji_landscape"] = layout;
}

- (id)getKBPortraitLayout:(KBLayoutType)type
{
    if (type == kKBLettersType)     return self.layouts[@"letters"];
    if (type == kKBNumbersType)     return self.layouts[@"numbers"];
    if (type == kKBSymbolsType)     return self.layouts[@"symbols"];
    if (type == kKBSuggestionsType) return self.layouts[@"suggestions"];
    if (type == kKBEmojiType)       return self.layouts[@"emoji"];
    return nil;
}

- (id)getKBLandscapeLayout:(KBLayoutType)type
{
    if (type == kKBLettersType)     return self.layouts[@"letters_landscape"];
    if (type == kKBNumbersType)     return self.layouts[@"numbers_landscape"];
    if (type == kKBSymbolsType)     return self.layouts[@"symbols_landscape"];
    if (type == kKBSuggestionsType) return self.layouts[@"suggestions_landscape"];
    if (type == kKBEmojiType)       return self.layouts[@"emoji_landscape"];
    return nil;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"obj = %@ layouts[%@] "
            "currentLayoutType[%lu] layoutState[%lu]",
            [super description], self.layouts,
            (unsigned long)self.currentLayoutType, (unsigned long)self.layoutState];
}

@end
