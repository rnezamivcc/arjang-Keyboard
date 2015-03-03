//
//  KeyboardViewEventsHandler.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-01.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "KBData.h"

@protocol KeyboardViewEventsHandler

- (void)handleKeyboardPopup:(KBData *)data;
- (void)handleKeyboardData:(KBData *)data;
- (void)handleKeyboardSwipe:(KBData *)data;

@end