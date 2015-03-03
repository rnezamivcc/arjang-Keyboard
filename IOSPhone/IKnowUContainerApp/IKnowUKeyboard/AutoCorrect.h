//
//  AutoCorrect.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-11-12.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "PredictionEngine/autocorrectIOS.h"

@interface AutoCorrect : NSObject

- (void)replaceBuffers:(NSString *)beforeCursor afterCursor:(NSString *)afterCursor;
- (void)reverseCorrection:(int)si autocorrect:(NSString *)autocorrect rep:(NSString *)rep;
- (void)loadKeyboardLayout:(NSArray *)keypos;

@end
