//
//  SharedDefaults.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-11-19.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>

// Settings - Assistance --------------------------------------------------------------------

#define AUTO_CAPITALIZE_PREFERENCE @"auto_capitalize_preference"
#define AUTO_LEARN_PREFERENCE @"auto_learn_preference"
#define AUTO_CORRECT_PREFERENCE @"auto_correct_preference"
#define AUTO_CORRECT_INSERT_PREFERENCE @"auto_correct_insert_preference"
#define DOUBLE_SPACE_PREFERENCE @"double_space_preference"

// Settings - Keyboard Feedback -------------------------------------------------------------

#define SOUND_ON_KEYPRESS_PREFERENCE @"sound_on_keypress_preference"
#define HIGHLIGHTED_KEYS_PREFERENCE @"highlighted_keys_preference"
#define CORRECTION_INDICATION_PREFERENCE @"correction_indication_preference"

// Settings - Cloud Services ----------------------------------------------------------------

#define CLOUDSYNC_ONOFF_PREFERENCE @"cloudsync_onoff_preference"
#define CLOUDSYNC_SESSIONTOKEN_PREFERENCE @"cloudsync_sessiontoken_preference"

// Settings - Personalization ---------------------------------------------------------------

#define SWIPE_TYPING_PREFERENCE @"swipe_preference"
#define PHRASE_PREDICTION_PREFERENCE @"phrase_predictions_preference"
#define LONG_PRESS_PREFERENCE @"long_press_preference"
#define WORD_DELETE_PREFERENCE @"word_delete_preference"
#define KEYBOARD_THEME_PREFERENCE @"keyboard_theme_preference"
#define REACH_PREFERENCE @"reach_preference"

// Location ---------------------------------------------------------------------------------

#define APARTMENT_NUMBER_PREFERENCE @"apartment_number_preference"
#define ADDRESS_PREFERENCE @"address_preference"
#define CITY_PREFERENCE @"city_preference"
#define PROVINCE_STATE_PREFERENCE @"province_state_preference"
#define COUNTRY_PREFERENCE @"country_preference"
#define POSTAL_ZIP_CODE_PREFERENCE @"postal_zip_code_preference"

// Dictionary -------------------------------------------------------------------------------

#define DICTIONARY_PREFERENCE @"dictionary_preference"

@interface SharedDefaults : NSObject

+ (void)registerDefaultSettings:(NSUserDefaults *)userDefaults;

@end
