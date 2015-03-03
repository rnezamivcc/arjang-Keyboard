//
//  Theme.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-10-06.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "Theme.h"

@implementation Theme

NSXMLParser *xmlThemeParser = nil;

- (id)initWithXmlFile:(NSString *)fileName
{
    self = [super init];
    
    if (self)
    {
        NSString *filePath = [[NSBundle mainBundle] pathForResource:fileName ofType: @"xml"];
        NSInputStream *stream = [NSInputStream inputStreamWithFileAtPath:filePath];
        xmlThemeParser = [[NSXMLParser alloc] initWithStream:stream];
        [xmlThemeParser setDelegate:self];
        
        return self;
    }
    
    return nil;
}

- (void)setup
{
    if (xmlThemeParser)
    {
        [xmlThemeParser parse];
    }
}

#pragma mark - NSXMLParserDelegate

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName
  namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName attributes:(NSDictionary *)attributeDict
{
    if ([elementName isEqualToString:@"item"])
    {
        NSString *name = [attributeDict objectForKey:@"name"];
        NSString *value = [attributeDict objectForKey:@"value"];
        
        if ([name isEqualToString:@"keyColor"])
        {
            self.keyColor = (value != nil) ? [self intFromHexString:value] : 0xFF4f4f4f;
        }
        else if ([name isEqualToString:@"keyPressedColor"])
        {
            self.keyPressedColor = (value != nil) ? [self intFromHexString:value] : 0xFFd3d3d3;
        }
        else if ([name isEqualToString:@"keyDarkColor"])
        {
            self.keyDarkColor = (value != nil) ? [self intFromHexString:value] : 0xFF2f2f2f;
        }
        else if ([name isEqualToString:@"backgroundColor"])
        {
            self.backgroundColor = (value != nil) ? [self intFromHexString:value] : 0xFF000000;
        }
        else if ([name isEqualToString:@"upperIconColor"])
        {
            self.keyUpperIconColor = (value != nil) ? [self intFromHexString:value] : 0xFFFFFF00;
        }
        else if ([name isEqualToString:@"keyColorMoreThan5"])
        {
            self.keyColorMoreThanFive = (value != nil) ? [self intFromHexString:value] : 0xFF007399;
        }
        else if ([name isEqualToString:@"keyColorLessThan5"])
        {
            self.keyColorLessThanFive = (value != nil) ? [self intFromHexString:value] : 0xFF009926;
        }
        else if ([name isEqualToString:@"keyTextColor"])
        {
            self.keyTextColor = (value != nil) ? [self intFromHexString:value] : 0xFFFFFFFF;
        }
        else if ([name isEqualToString:@"candidateBackground"])
        {
            self.candidateBackgroundColor = (value != nil) ? [self intFromHexString:value] : 0xFF000000;
        }
        else if ([name isEqualToString:@"candidateTextColor"])
        {
            self.candidateTextColor = (value != nil) ? [self intFromHexString:value] : 0xFFFFFFFF;
        }
        else if ([name isEqualToString:@"candidateSelectedColor"])
        {
            self.candidateSelectedColor = (value != nil) ? [self intFromHexString:value] : 0xFF0000FF;
        }
        else if ([name isEqualToString:@"candidateAddWordColor"])
        {
            self.candidateAddWordColor = (value != nil) ? [self intFromHexString:value] : 0xFF228b22;
        }
        else if ([name isEqualToString:@"candidateDeleteWordColor"])
        {
            self.candidateDeleteWordColor = (value != nil) ? [self intFromHexString:value] : 0xFFb22222;
        }
        else if ([name isEqualToString:@"highestPriorityColor"])
        {
            self.candidateHighestPriorityColor = (value != nil) ? [self intFromHexString:value] : 0xFFb22222;
        }
        else if ([name isEqualToString:@"style"])
        {
            self.keyStyle = (value != nil) ? [value integerValue] : 0;
        }
        else if ([name isEqualToString:@"stroke"])
        {
            self.borderStroke = (value != nil) ? [value integerValue] : 1;
        }
        else if ([name isEqualToString:@"keyShadowColor"])
        {
            self.keyShadowColor = (value != nil) ? [self intFromHexString:value] : 0xDD000000;
        }
        else if ([name isEqualToString:@"useGradient"])
        {
            self.useGradient = (value != nil) ? [value boolValue] : false;
        }
        else if ([name isEqualToString:@"gradientDirection"])
        {
            self.gradientDirection = (value != nil) ? [value integerValue] : 0;
        }
        else if ([name isEqualToString:@"cornerRadiusX"])
        {
            self.cornerRadiusX = (value != nil) ? [value integerValue] : 0;
        }
        else if ([name isEqualToString:@"cornerRadiusY"])
        {
            self.cornerRadiusY = (value != nil) ? [value integerValue] : 0;
        }
        else if ([name isEqualToString:@"searchItemColor"])
        {
            self.searchItemColor = (value != nil) ? [self intFromHexString:value] : 0xFF000000;
        }
        else if ([name isEqualToString:@"searchItemPressedColor"])
        {
            self.searchItemPressedColor = (value != nil) ? [self intFromHexString:value] : 0xFF000000;
        }
        else if ([name isEqualToString:@"searchBackColor"])
        {
            self.searchItemBackColor = (value != nil) ? [self intFromHexString:value] : 0xFF000000;
        }
        else if ([name isEqualToString:@"searchItemTextColor"])
        {
            self.searchItemTextColor = (value != nil) ? [self intFromHexString:value] : 0xFF000000;
        }
        else if ([name isEqualToString:@"previewBackgroundColor"])
        {
            self.previewBackgroundColor = (value != nil) ? [self intFromHexString:value] : 0xFF000000;
        }
        else if ([name isEqualToString:@"previewTextColor"])
        {
            self.previewTextColor = (value != nil) ? [self intFromHexString:value] : 0xFFFFFFFF;
        }
        else if ([name isEqualToString:@"previewBorderColor"])
        {
            self.previewBorderColor = (value != nil) ? [self intFromHexString:value] : 0xFF000000;
        }
    }
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName
  namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
    if ([elementName isEqualToString:@"item"])
    {
    }
}

// Error handling
-(void)parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError
{
    IKU_log(@"XMLParser error: %@", [parseError localizedDescription]);
}

-(void)parser:(NSXMLParser *)parser validationErrorOccurred:(NSError *)validationError
{
    IKU_log(@"XMLParser error: %@", [validationError localizedDescription]);
}

- (NSInteger)intFromHexString:(NSString *)hexStr
{
    unsigned int hexInt = 0;
    
    NSScanner *scanner = [NSScanner scannerWithString:hexStr];
    [scanner setCharactersToBeSkipped:[NSCharacterSet characterSetWithCharactersInString:@"#"]];
    [scanner scanHexInt:&hexInt];
    
    return hexInt;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"%@ "
            "keyColor[%08lX] "
            "keyPressedColor[%08lX] "
            "keyDarkColor[%08lX] "
            "backgroundColor[%08lX] "
            "keyUpperIconColor[%08lX] "
            "keyColorMoreThanFive[%08lX] "
            "keyColorLessThanFive[%08lX] "
            "keyTextColor[%08lX] "
            "candidateBackgroundColor[%08lX] "
            "candidateTextColor[%08lX] "
            "candidateSelectedColor[%08lX] "
            "candidateAddWordColor[%08lX] "
            "candidateDeleteWordColor[%08lX] "
            "candidateHighestPriorityColor[%08lX] "
            "keyStyle[%ld] "
            "borderStroke[%ld] "
            "keyShadowColor[%08lX] "
            "useGradient[%d] "
            "gradientDirection[%ld] "
            "cornerRadiusX[%ld] "
            "cornerRadiusY[%ld] "
            "searchItemColor[%08lX] "
            "searchItemPressedColor[%08lX] "
            "searchItemBackColor[%08lX] "
            "searchItemTextColor[%08lX] "
            "previewBackgroundColor[%08lX] "
            "previewTextColor[%08lX] "
            "previewBorderColor[%08lX]",
            [super description],
            (long)self.keyColor,
            (long)self.keyPressedColor,
            (long)self.keyDarkColor,
            (long)self.backgroundColor,
            (long)self.keyUpperIconColor,
            (long)self.keyColorMoreThanFive,
            (long)self.keyColorLessThanFive,
            (long)self.keyTextColor,
            (long)self.candidateBackgroundColor,
            (long)self.candidateTextColor,
            (long)self.candidateSelectedColor,
            (long)self.candidateAddWordColor,
            (long)self.candidateDeleteWordColor,
            (long)self.candidateHighestPriorityColor,
            (long)self.keyStyle,
            (long)self.borderStroke,
            (long)self.keyShadowColor,
            self.useGradient,
            (long)self.gradientDirection,
            (long)self.cornerRadiusX,
            (long)self.cornerRadiusY,
            (long)self.searchItemColor,
            (long)self.searchItemPressedColor,
            (long)self.searchItemBackColor,
            (long)self.searchItemTextColor,
            (long)self.previewBackgroundColor,
            (long)self.previewTextColor,
            (long)self.previewBorderColor];
}

@end
