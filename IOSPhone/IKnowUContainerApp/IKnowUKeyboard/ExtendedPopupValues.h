//
//  ExtendedPopupValues.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-30.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ExtendedPopupValues : NSObject

@property (strong, nonatomic) NSMutableString *theChar;
@property (strong, nonatomic) NSMutableArray *keyChars;
@property (strong, nonatomic) NSMutableString *defaultChar;

- (id)initWithChr:(NSString *)chr;
- (void)addKeyChar:(NSString *)chr isDefault:(NSString *)isDefault;
- (NSArray *)getKeyChars;
- (NSString *)getMainChar;
- (NSString *)getDefaultChar;

@end
