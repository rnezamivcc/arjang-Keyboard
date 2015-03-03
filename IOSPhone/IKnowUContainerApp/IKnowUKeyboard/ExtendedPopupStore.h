//
//  ExtendedPopupStore.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-30.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ExtendedPopupValues.h"

@interface ExtendedPopupStore : NSObject <NSXMLParserDelegate>

@property (strong, nonatomic) NSMutableDictionary *popupValues;
@property (strong, nonatomic) ExtendedPopupValues *currentPopupValues;

- (id)initWithXmlFile:(NSString *)fileName;
- (void)setup;
- (ExtendedPopupValues *)getValues:(NSString *)key;

@end
