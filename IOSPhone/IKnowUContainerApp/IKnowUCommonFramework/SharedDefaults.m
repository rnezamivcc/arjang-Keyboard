//
//  SharedDefaults.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-11-19.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "SharedDefaults.h"

@implementation SharedDefaults

// This selector is called in the container app and the keyboard extension.
// The keyboard extension needs to call this selector in case the container
// app is never opened.
+ (void)registerDefaultSettings:(NSUserDefaults *)userDefaults
{
    // Set default values for preferences.
    NSDictionary *appDefaults = [NSDictionary dictionaryWithObjectsAndKeys:
                                 // Assistance
                                 @YES, AUTO_CAPITALIZE_PREFERENCE,
                                 @YES, AUTO_LEARN_PREFERENCE,
                                 @YES, AUTO_CORRECT_PREFERENCE,
                                 @YES, AUTO_CORRECT_INSERT_PREFERENCE,
                                 @YES, DOUBLE_SPACE_PREFERENCE,
                                 
                                 // Keyboard Feedback
                                 @YES, SOUND_ON_KEYPRESS_PREFERENCE,
                                 @YES, HIGHLIGHTED_KEYS_PREFERENCE,
                                 @NO, CORRECTION_INDICATION_PREFERENCE,
                                 
                                 // Cloud Services
                                 @NO, CLOUDSYNC_ONOFF_PREFERENCE,
                                 @"", CLOUDSYNC_SESSIONTOKEN_PREFERENCE,
                                 
                                 // Personalization
                                 @NO, SWIPE_TYPING_PREFERENCE,
                                 @YES, PHRASE_PREDICTION_PREFERENCE,
                                 [NSString stringWithFormat:@"%f", LONG_PRESS_TIMEOUT_MIN], LONG_PRESS_PREFERENCE,
                                 [NSString stringWithFormat:@"%f", DELETE_WORD_TIMEOUT_MIN], WORD_DELETE_PREFERENCE,
                                 @"Zorin", KEYBOARD_THEME_PREFERENCE,
                                 @YES, REACH_PREFERENCE,
                                 nil];
    
    [userDefaults registerDefaults:appDefaults];
}

@end
