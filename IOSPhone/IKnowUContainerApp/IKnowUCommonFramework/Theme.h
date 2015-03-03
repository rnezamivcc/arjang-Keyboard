//
//  Theme.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-10-06.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Theme : NSObject <NSXMLParserDelegate>

@property (nonatomic) NSInteger keyColor;
@property (nonatomic) NSInteger keyPressedColor;
@property (nonatomic) NSInteger keyDarkColor;
@property (nonatomic) NSInteger backgroundColor;
@property (nonatomic) NSInteger keyUpperIconColor;
@property (nonatomic) NSInteger keyColorMoreThanFive;
@property (nonatomic) NSInteger keyColorLessThanFive;
@property (nonatomic) NSInteger keyTextColor;
@property (nonatomic) NSInteger candidateBackgroundColor;
@property (nonatomic) NSInteger candidateTextColor;
@property (nonatomic) NSInteger candidateSelectedColor;
@property (nonatomic) NSInteger candidateAddWordColor;
@property (nonatomic) NSInteger candidateDeleteWordColor;
@property (nonatomic) NSInteger candidateHighestPriorityColor;
@property (nonatomic) NSInteger keyStyle;
@property (nonatomic) NSInteger borderStroke;
@property (nonatomic) NSInteger keyShadowColor;
@property (nonatomic) BOOL      useGradient;
@property (nonatomic) NSInteger gradientDirection;
@property (nonatomic) NSInteger cornerRadiusX;
@property (nonatomic) NSInteger cornerRadiusY;
@property (nonatomic) NSInteger searchItemColor;
@property (nonatomic) NSInteger searchItemPressedColor;
@property (nonatomic) NSInteger searchItemBackColor;
@property (nonatomic) NSInteger searchItemTextColor;
@property (nonatomic) NSInteger previewBackgroundColor;
@property (nonatomic) NSInteger previewTextColor;
@property (nonatomic) NSInteger previewBorderColor;

- (id)initWithXmlFile:(NSString *)fileName;
- (void)setup;

@end
