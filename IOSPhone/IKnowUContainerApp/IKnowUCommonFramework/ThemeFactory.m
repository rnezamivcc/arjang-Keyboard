//
//  ThemeFactory.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-10-06.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "ThemeFactory.h"

@interface ThemeFactory ()

@property (strong, nonatomic) NSDictionary *dictionary;
@property (strong, nonatomic) NSMutableDictionary *themes;

@end

@implementation ThemeFactory

+ (ThemeFactory *)defaultThemeFactory
{
    static ThemeFactory *sharedThemeFactory = nil;
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        sharedThemeFactory = [[self alloc] init];
    });    
    return sharedThemeFactory;
}

+ (NSArray *)allKeys
{   
    static NSArray *keys;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        keys = @[ // Each of the key names maps to a theme
                 @"iKnowU",              // -> theme1
                 @"Holo",                // -> theme15
                 @"iPple",               // -> theme16
                 @"Sophisticated",       // -> theme11
                 @"Dark",                // -> theme12
                 @"Zorin",               // -> theme13
                 @"Midnight",            // -> theme2
                 @"Neon Green",          // -> theme3
                 @"Rouge",               // -> theme4
                 @"Cool Blue",           // -> theme5
                 @"Neon Orange",         // -> theme6
                 @"Black and White",     // -> theme7
                 @"Light",               // -> theme8
                 @"Pretty in Pink",      // -> theme9
                 @"Bumblebee",           // -> theme10
                 @"Retro",               // -> theme14
                 @"Crystal Gloss",       // -> theme17
                 ];
    });
    return keys;
}

- (id)init
{
    self = [super init];
    if (self)
    {
        NSArray *objects = [NSArray arrayWithObjects:
                            @"theme1",
                            @"theme15",
                            @"theme16",
                            @"theme11",
                            @"theme12",
                            @"theme13",
                            @"theme2",
                            @"theme3",
                            @"theme4",
                            @"theme5",
                            @"theme6",
                            @"theme7",
                            @"theme8",
                            @"theme9",
                            @"theme10",
                            @"theme14",
                            @"theme17",
                            nil];
        self.dictionary = [NSDictionary dictionaryWithObjects:objects forKeys:[ThemeFactory allKeys]];
        
        self.themes = [[NSMutableDictionary alloc] init];
        
        for (NSInteger i = 0; i < [objects count]; i++)
        {
            Theme *theme = [[Theme alloc] initWithXmlFile:[objects objectAtIndex:i]];
            [theme setup];
            [self.themes setObject:theme forKey:[objects objectAtIndex:i]];
        }
        
        IKU_log(@"init : [%ld] themes loaded", (unsigned long)[self.themes count]);
    }
    return self;
}

- (Theme *)getTheme:(NSString *)key
{
    NSString *themeId = [self.dictionary objectForKey:key];
    Theme *theme = [self.themes objectForKey:themeId];
    if (theme == nil)
    {
        theme = [self.themes objectForKey:@"theme13"];
    }
    return theme;
}

@end
