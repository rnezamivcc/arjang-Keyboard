//
//  KBData.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-17.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface KBData : NSObject

typedef NS_ENUM(NSUInteger, PopupState)
{
    kDoNothing,
    kOpenStandardPopup,
    kOpenExtendedPopup,
    kClosePopup
};

typedef NS_ENUM(NSUInteger, SwipeAction)
{
    kSwipeNone,
    kSwipeStartTrail,
    kSwipeEndTrail,
    kSwipeLeft,
    kSwipeRight
};

@property (strong, nonatomic) NSMutableString *s;
@property (strong, nonatomic) id sender;
@property (nonatomic) Boolean isPhrase;
@property (nonatomic) Boolean isPrediction;
@property (nonatomic) Boolean isLongPressedStart;
@property (nonatomic) Boolean isLongPressedEnd;
@property (nonatomic) Boolean isShifted;
@property (nonatomic) Boolean isShiftLocked;
@property (nonatomic) Boolean isEmoji;
@property (nonatomic) Boolean isBackSpace;
@property (nonatomic) NSInteger tag;
@property (nonatomic) PopupState popupState;
@property (nonatomic) SwipeAction swipeAction;

- (void)reset;

@end
