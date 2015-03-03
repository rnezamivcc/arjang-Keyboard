//
//  IKUMacros.h
//
//  Created by Chris Bateman on 2014-07-08.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#define SPACE @" "
#define NEW_LINE @"\n" 

#ifdef DEBUG
#define IKU_log( s, ... ) NSLog( @"IKULog <%@:%d> %@", [[NSString stringWithUTF8String:__FILE__] lastPathComponent], __LINE__,  [NSString stringWithFormat:(s), ##__VA_ARGS__] )
#else
#define IKU_log( s, ... )
#endif

#define RGB(r, g, b) [UIColor colorWithRed:r/255.0 green:g/255.0 blue:b/255.0 alpha:1]
#define RGBA(r, g, b, a) [UIColor colorWithRed:r/255.0 green:g/255.0 blue:b/255.0 alpha:a]
#define UIColorFromRGB(rgbValue) [UIColor colorWithRed:((float)((rgbValue & 0xFF0000) >> 16))/255.0 green:((float)((rgbValue & 0xFF00) >> 8))/255.0 blue:((float)(rgbValue & 0xFF))/255.0 alpha:1.0]
#define UIColorFromRGBWithAlpha(rgbValue, alphaValue) [UIColor colorWithRed:((float)((rgbValue & 0xFF0000) >> 16))/255.0 green:((float)((rgbValue & 0xFF00) >> 8))/255.0 blue:((float)(rgbValue & 0xFF))/255.0 alpha:alphaValue]

#define RADIANS_TO_DEGREES(radians) ((radians) * (180.0 / M_PI))
#define DEGREES_TO_RADIANS(angle) ((angle) / 180.0 * M_PI)

#define IPHONE_KEYBOARD_PORTRAIT_HEIGHT 260
#define IPHONE_KEYBOARD_LANDSCAPE_HEIGHT 216
#define IPAD_KEYBOARD_PORTRAIT_HEIGHT 264
#define IPAD_KEYBOARD_LANDSCAPE_HEIGHT 264

#define IPHONE_SUGGESTIONS_HEIGHT 54
#define IPAD_SUGGESTIONS_HEIGHT 70

#define BUTTON_CORNER_RADIUS 4.0

#define POPUP_BUTTON_HEIGHT 30.0
#define POPUP_BUTTON_WIDTH 30.0
#define POPUP_BUTTON_SPACING 7.0

#define POPUP_IPAD_BUTTON_HEIGHT 40.0
#define POPUP_IPAD_BUTTON_WIDTH 74.0
#define POPUP_IPAD_BUTTON_SPACING 7.0

#define MOVEMENT_THRESHHOLD 150.0
#define SWIPE_TRAIL_THRESHHOLD 50.0

#define REGEX_EMAIL @"[_A-Za-z0-9-]+(\\.[_A-Za-z0-9-]+)*@[A-Za-z0-9]+(\\.[A-Za-z0-9]+)*(\\.[A-Za-z]{2,})"
#define REGEX_PHONE_NUMBER @"(\\d{3}-?){1,2}\\d{4}"
#define REGEX_PROPER_NOUN @"[A-Z][a-z]+"

#define MINI_APP_BAR_HEIGHT 16.0
#define MINI_APP_MAIN_BAR_HEIGHT 30.0

#define CONTACT_LIST_IMAGE_HEIGHT 65
#define CONTACT_LIST_CELL_HEIGHT 90
#define CONTACT_DETAIL_IMAGE_HEIGHT 60

#define CONTACT_LIST_IPAD_IMAGE_HEIGHT 74
#define CONTACT_LIST_IPAD_CELL_HEIGHT 90
#define CONTACT_DETAIL_IPAD_IMAGE_HEIGHT 120

#define SEARCH_LIST_CELL_HEIGHT 90

#define SEARCH_LIST_IPAD_CELL_HEIGHT 90

#define LONG_PRESS_TIMEOUT_MIN 200.0
#define LONG_PRESS_TIMEOUT_MAX 500.0

#define DELETE_WORD_TIMEOUT_MIN 200.0
#define DELETE_WORD_TIMEOUT_MAX 500.0
