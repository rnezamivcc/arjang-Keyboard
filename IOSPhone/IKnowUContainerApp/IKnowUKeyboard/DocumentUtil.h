//
//  DocumentUtil.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-14.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface DocumentUtil : NSObject

+ (NSString *)replaceFirstNewLine:(NSString *)original;
+ (NSMutableCharacterSet *)getWordSeparators;
+ (NSMutableCharacterSet *)getPunctuation;
+ (NSInteger)lastIndexOf:(NSString *)s1 in:(NSString *)s2;
+ (BOOL)shouldCapitalize:(NSObject < UITextDocumentProxy > *)textDocumentProxy;
+ (NSString *)autocapitalizeChar:(NSString *)s textDocumentProxy:(NSObject < UITextDocumentProxy > *)textDocumentProxy;
+ (NSString *)autocapitalizeWord:(NSString *)word textDocumentProxy:(NSObject < UITextDocumentProxy > *)textDocumentProxy;
+ (NSString *)autocapitalizePhrase:(NSString *)phrase textDocumentProxy:(NSObject < UITextDocumentProxy > *)textDocumentProxy;
+ (NSString *)getWordsBeforeCursor:(int)numWords textDocumentProxy:(NSObject < UITextDocumentProxy > *)textDocumentProxy;
+ (NSString *)getLastWord:(NSObject < UITextDocumentProxy > *)textDocumentProxy;
+ (NSString *)getLastWord:(bool)includeWordSeps includeCurrent:(bool)includeCurrent textDocumentProxy:(NSObject < UITextDocumentProxy > *)textDocumentProxy;
+ (NSString *)convertUnicodeToString:(NSString *)str;
+ (bool)string:(NSString *)string containsSubstring:(NSString *)substring;

@end