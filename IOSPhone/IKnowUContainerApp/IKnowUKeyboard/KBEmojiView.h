//
//  KBEmojiView.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-19.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "KeyboardView.h"

@interface KBEmojiView : KeyboardView

- (void)initialize;
- (IBAction)buttonPressed:(id)sender;
- (IBAction)buttonLongPressed:(id)sender;

@end
