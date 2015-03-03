//
//  ExtendedPopupStore.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-30.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "DocumentUtil.h"
#import "ExtendedPopupStore.h"

@implementation ExtendedPopupStore

NSXMLParser *xmlParser = nil;

- (id)initWithXmlFile:(NSString *)fileName
{
    self = [super init];
    
    if (self)
    {
        self.popupValues = [[NSMutableDictionary alloc] init];
        
        NSString *filePath = [[NSBundle mainBundle] pathForResource:fileName ofType: @"xml"];
        NSInputStream *stream = [NSInputStream inputStreamWithFileAtPath:filePath];
        xmlParser = [[NSXMLParser alloc] initWithStream:stream];
        [xmlParser setDelegate:self];
        
        return self;
    }
    
    return nil;
}

- (void)setup
{
    if (xmlParser)
    {
        [xmlParser parse];
    }
}

- (ExtendedPopupValues *)getValues:(NSString *)key
{
    return [self.popupValues objectForKey:key];
}

#pragma mark - NSXMLParserDelegate

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName
  namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName attributes:(NSDictionary *)attributeDict
{
    if ([elementName isEqualToString:@"character"])
    {
        NSString *value = [attributeDict objectForKey:@"value"];
        self.currentPopupValues = [[ExtendedPopupValues alloc] initWithChr:value];
        [self.popupValues setObject:self.currentPopupValues forKey:value];
    }
    else if ([elementName isEqualToString:@"key"])
    {
        NSString *value = [attributeDict objectForKey:@"value"];
        NSString *defaultValue = [attributeDict objectForKey:@"default"];
        if ([value hasPrefix:@"\\u"])
        {
            NSString *convertedString = [DocumentUtil convertUnicodeToString:value];
            [self.currentPopupValues addKeyChar:convertedString isDefault:defaultValue];
        }
        else
        {
            [self.currentPopupValues addKeyChar:value isDefault:defaultValue];
        }        
    }
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName
  namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
    if ([elementName isEqualToString:@"character"])
    {
    }
}

// error handling
-(void)parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError
{
    IKU_log(@"XMLParser error: %@", [parseError localizedDescription]);
}

-(void)parser:(NSXMLParser *)parser validationErrorOccurred:(NSError *)validationError
{
    IKU_log(@"XMLParser error: %@", [validationError localizedDescription]);
}

@end
