//
//  ThemeFactory.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-10-06.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Theme.h"

@interface ThemeFactory : NSObject

+ (ThemeFactory *)defaultThemeFactory;
+ (NSArray *)allKeys;
- (Theme *)getTheme:(NSString *)key;

@end
