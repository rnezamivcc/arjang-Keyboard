//
//  DocumentUtil.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-14.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "DocumentUtil.h"

@implementation DocumentUtil

NSMutableCharacterSet *wordSeparators = nil;
NSMutableCharacterSet *punctuation = nil;

+ (NSString *)replaceFirstNewLine:(NSString *)original
{
    NSMutableString *newString = [NSMutableString stringWithString:original];
    
    NSRange foundRange = [original rangeOfString:@"\n"];
    if (foundRange.location != NSNotFound)
    {
        [newString replaceCharactersInRange:foundRange withString:@""];
    }
    
    return newString;
}

+ (NSMutableCharacterSet *)getWordSeparators
{
    if (wordSeparators == nil)
    {
        wordSeparators = [[NSMutableCharacterSet alloc] init];
        [wordSeparators addCharactersInString:@" .,;:!?\n()[]*&@{}/<>_+=|'"];
    }
    
    return wordSeparators;
}

+ (NSMutableCharacterSet *)getPunctuation
{
    if (punctuation == nil)
    {
        punctuation = [[NSMutableCharacterSet alloc] init];
        [punctuation addCharactersInString:@".,!?"];
    }
    
    return punctuation;
}

+ (NSInteger)lastIndexOf:(NSString *)s1 in:(NSString *)s2
{
    NSRange range = [s2 rangeOfString:s1 options:NSBackwardsSearch];
    return (range.location == NSNotFound) ? -1 : range.location;
}

+ (BOOL)shouldCapitalize:(NSObject < UITextDocumentProxy > *)textDocumentProxy
{
    switch (textDocumentProxy.autocapitalizationType)
    {
        case UITextAutocapitalizationTypeNone:
        {
            IKU_log(@"autocapitalizeChar : type[UITextAutocapitalizationTypeNone]");
            
            return false;
        }
            
        case UITextAutocapitalizationTypeAllCharacters:
        {
            IKU_log(@"autocapitalizeChar : type[UITextAutocapitalizationTypeAllCharacters]");
            
            return true;
        }
            
        case UITextAutocapitalizationTypeWords:
        {
            IKU_log(@"autocapitalizeChar : type[UITextAutocapitalizationTypeWords]");
            
            NSString *precedingContext = textDocumentProxy.documentContextBeforeInput;
            unichar last = [precedingContext characterAtIndex:[precedingContext length] - 1];
            if ((precedingContext.length <= 0) || [[NSCharacterSet whitespaceAndNewlineCharacterSet] characterIsMember:last])
            {
                return true;
            }
            return false;
        }
            
        case UITextAutocapitalizationTypeSentences:
        {
            IKU_log(@"autocapitalizeChar : type[UITextAutocapitalizationTypeSentences]");
            
            NSString *precedingContext = textDocumentProxy.documentContextBeforeInput;
            NSString *trimmedString = [precedingContext stringByTrimmingCharactersInSet:
                                       [NSCharacterSet whitespaceAndNewlineCharacterSet]];
            if ([trimmedString length] == 0)
            {
                return true;
            }
            
            NSMutableCharacterSet *characterSet = [[NSMutableCharacterSet alloc] init];
            [characterSet addCharactersInString:@".?!"];
            unichar last = [trimmedString characterAtIndex:[trimmedString length] - 1];
            if ([characterSet characterIsMember:last])
            {
                return true;
            }
            
            return false;
        }
    }
    
    IKU_log(@"autocapitalizeChar : type[unknown]");
    
    return false;
}

+ (NSString *)autocapitalizeChar:(NSString *)s textDocumentProxy:(NSObject < UITextDocumentProxy > *)textDocumentProxy
{
    switch (textDocumentProxy.autocapitalizationType)
    {
        case UITextAutocapitalizationTypeNone:
        {
            IKU_log(@"autocapitalizeChar : type[UITextAutocapitalizationTypeNone]");
            
            return [s lowercaseString];
        }
            
        case UITextAutocapitalizationTypeAllCharacters:
        {
            IKU_log(@"autocapitalizeChar : type[UITextAutocapitalizationTypeAllCharacters]");
            
            return s;
        }
            
        case UITextAutocapitalizationTypeWords:
        {
            IKU_log(@"autocapitalizeChar : type[UITextAutocapitalizationTypeWords]");
            
            NSString *precedingContext = textDocumentProxy.documentContextBeforeInput;
            unichar last = [precedingContext characterAtIndex:[precedingContext length] - 1];
            if ((precedingContext.length <= 0) ||
                [[NSCharacterSet whitespaceAndNewlineCharacterSet] characterIsMember:last])
            {
                return s;
            }
            return [s lowercaseString];
        }
            
        case UITextAutocapitalizationTypeSentences:
        {
            IKU_log(@"autocapitalizeChar : type[UITextAutocapitalizationTypeSentences]");
            
            NSString *precedingContext = textDocumentProxy.documentContextBeforeInput;
            NSString *trimmedString = [precedingContext stringByTrimmingCharactersInSet:
                                       [NSCharacterSet whitespaceAndNewlineCharacterSet]];
            if ([trimmedString length] == 0)
            {
                return s;
            }
            
            NSMutableCharacterSet *characterSet = [[NSMutableCharacterSet alloc] init];
            [characterSet addCharactersInString:@".?!"];
            unichar last = [trimmedString characterAtIndex:[trimmedString length] - 1];
            if ([characterSet characterIsMember:last])
            {
                return s;
            }
            
            return [s lowercaseString];
        }
    }
    
    IKU_log(@"autocapitalizeChar : type[unknown]");
     
    return nil;
}

+ (NSString *)autocapitalizeWord:(NSString *)word textDocumentProxy:(NSObject < UITextDocumentProxy > *)textDocumentProxy
{
    switch (textDocumentProxy.autocapitalizationType)
    {
        case UITextAutocapitalizationTypeNone:
        {
            IKU_log(@"autocapitalizeWord : type[UITextAutocapitalizationTypeNone]");
            
            return word;
        }
            
        case UITextAutocapitalizationTypeAllCharacters:
        {
            IKU_log(@"autocapitalizeWord : type[UITextAutocapitalizationTypeAllCharacters]");
            
            return [word uppercaseString];
        }
            
        case UITextAutocapitalizationTypeWords:
        {
            IKU_log(@"autocapitalizeWord : type[UITextAutocapitalizationTypeWords]");
            
            return [word capitalizedString];
        }
            
        case UITextAutocapitalizationTypeSentences:
        {
            IKU_log(@"autocapitalizeWord : type[UITextAutocapitalizationTypeSentences]");
            
            return [word capitalizedString];
        }
    }
    
    IKU_log(@"autocapitalizeWord : type[unknown]");
    return nil;
}

+ (NSString *)autocapitalizePhrase:(NSString *)phrase textDocumentProxy:(NSObject < UITextDocumentProxy > *)textDocumentProxy
{
    switch (textDocumentProxy.autocapitalizationType)
    {
        case UITextAutocapitalizationTypeNone:
        {
            IKU_log(@"autocapitalizePhrase : type[UITextAutocapitalizationTypeNone]");
            
            return phrase;
        }
            
        case UITextAutocapitalizationTypeAllCharacters:
        {
            IKU_log(@"autocapitalizePhrase : type[UITextAutocapitalizationTypeAllCharacters]");
            
            return [phrase uppercaseString];
        }
            
        case UITextAutocapitalizationTypeWords:
        {
            IKU_log(@"autocapitalizePhrase : type[UITextAutocapitalizationTypeWords]");
            
            return [phrase capitalizedString];
        }
            
        case UITextAutocapitalizationTypeSentences:
        {
            IKU_log(@"autocapitalizePhrase : type[UITextAutocapitalizationTypeSentences]");
            
            NSString *precedingContext = textDocumentProxy.documentContextBeforeInput;
            NSString *trimmedString = [precedingContext stringByTrimmingCharactersInSet:
                                       [NSCharacterSet whitespaceAndNewlineCharacterSet]];
            if ([trimmedString length] > 0)
            {
                return phrase;
            }
            
            NSString *uppercase = [[phrase substringToIndex:1] uppercaseString];
            NSString *lowercase = [[phrase substringFromIndex:1] lowercaseString];
            return [uppercase stringByAppendingString:lowercase];
        }
    }
    
    IKU_log(@"autocapitalizePhrase : type[unknown]");
    return nil;
}

// Get the specified number of words prior to the current cursor position.
// This will generally only be called when a correction inserted 2 or more words
// and the user wants to change it back to one.
//
// This function will grab the current word being typed if any, and
// count that as a word to be returned
//
// numWords - the number of words to return
// return the words found
+ (NSString *)getWordsBeforeCursor:(int)numWords textDocumentProxy:(NSObject < UITextDocumentProxy > *)textDocumentProxy
{
    NSString *precedingContext = textDocumentProxy.documentContextBeforeInput;
    
    NSMutableString *words = [[NSMutableString alloc] init];
    NSArray *wordsAndEmptyStrings = [precedingContext componentsSeparatedByCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
    NSArray *wordsAR = [wordsAndEmptyStrings filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"length > 0"]];
    
    NSInteger start = [wordsAR count] - 1;
    NSInteger stop = (numWords <= [wordsAR count]) ? [wordsAR count] - numWords : 0;
    
    for (NSInteger i = start; i >= stop; i--)
    {
        if (precedingContext.length > 0)
        {
            if ((i == start) && [precedingContext characterAtIndex:[precedingContext length] - 1] != ' ')
            {
                [words setString:[wordsAR objectAtIndex:i]];
            }
            else
            {
                [words insertString:[NSString stringWithFormat:@"%@ ", [wordsAR objectAtIndex:i]] atIndex:0];
            }
        }
    }
    
    return words;
}

// Gets the last full word before the current cursor position
//
// return the text found, or an empty string if any conditions are not met.
+ (NSString *)getLastWord:(NSObject < UITextDocumentProxy > *)textDocumentProxy
{
    NSString *precedingContext = textDocumentProxy.documentContextBeforeInput;
    NSInteger length = [precedingContext length];
    
    __block NSString *lastWord = nil;
    
    [precedingContext enumerateSubstringsInRange:NSMakeRange(0, [precedingContext length]) options:NSStringEnumerationByWords | NSStringEnumerationReverse usingBlock:^(NSString *substring, NSRange subrange, NSRange enclosingRange, BOOL *stop)
     {
         lastWord = substring;
         *stop = YES;
     }];
    
    if (lastWord != nil)
    {
        NSRange range = [precedingContext rangeOfString:lastWord options:NSBackwardsSearch];
        NSInteger lastWordEnd = range.location + range.length;
        if (lastWordEnd < length)
        {
            NSRange newRange = NSMakeRange(lastWordEnd, length - lastWordEnd);
            NSString *endPart = [precedingContext substringWithRange:newRange];
            
            return [lastWord stringByAppendingFormat:@"%@", endPart];
        }
        return lastWord;
    }
    
    return @"";
}

// Gets the last full word before the current cursor position
//
// includeWordSeps - if true, back out if a word separator ('.' ',' '?' '!' etc.) is detected before a space
// includeCurrent - if true, return the current word being typed, otherwise look further back
// return the text found, or an empty string if any conditions are not met.
 + (NSString *)getLastWord:(bool)includeWordSeps includeCurrent:(bool)includeCurrent textDocumentProxy:(NSObject < UITextDocumentProxy > *)textDocumentProxy
{
    NSString *precedingContext = textDocumentProxy.documentContextBeforeInput;
    NSInteger length = [precedingContext length];
    
    if (length > 0)
    {
        // always check for spaces or new lines, we do not want these
        NSInteger spaceIndex = [DocumentUtil lastIndexOf:SPACE in:precedingContext];
        NSInteger newLineIndex = [DocumentUtil lastIndexOf:NEW_LINE in:precedingContext];
        NSInteger closest = spaceIndex > newLineIndex ? spaceIndex : newLineIndex; // closer of a new line or a space to the cursor
        
        // check to see where the cursor is, we do not want the current word being typed,
        // if this is the case, then substring that part out
        if ((spaceIndex > 0 || newLineIndex > 0) && (closest != (length - 1)))
        {
            if (includeWordSeps)
            {
                if (includeCurrent)
                {
                    return [precedingContext substringWithRange:NSMakeRange(closest + 1, length - (closest + 1))];
                }
                // if we are looking for word seps, try to see if the last character
                // is one. If it is, return an empty string
                NSString *lastChar = [precedingContext substringWithRange:NSMakeRange(length - 1, 1)];
                unichar last = [lastChar characterAtIndex:0];
                if ([[DocumentUtil getWordSeparators] characterIsMember:last])
                {
                    return @"";
                }
                else
                {
                    precedingContext = [precedingContext substringWithRange:NSMakeRange(0, closest)];
                }
            }
            else
            {
                if (includeCurrent)
                {
                    return [precedingContext substringWithRange:NSMakeRange(closest + 1, length - (closest + 1))];
                }
                else
                {
                    precedingContext = [precedingContext substringWithRange:NSMakeRange(0, closest)];
                }
            }
        }
        else if (spaceIndex < 0 && newLineIndex < 0)
        {
            if (includeCurrent)
            {
                return precedingContext;
            }
            else
            {
                precedingContext = @""; // this would mean the very first word in the box of text
            }
        }
        else if (spaceIndex == 0 || newLineIndex == 0)
        {
            return [precedingContext stringByTrimmingCharactersInSet:
                    [NSCharacterSet whitespaceAndNewlineCharacterSet]];
        }
        
        spaceIndex = [DocumentUtil lastIndexOf:SPACE in:precedingContext];
        newLineIndex = [DocumentUtil lastIndexOf:NEW_LINE in:precedingContext];
        closest = spaceIndex > newLineIndex ? spaceIndex : newLineIndex;
        
        length = precedingContext.length;
        if (length > 0)
        {
            if ((spaceIndex > 0 || newLineIndex > 0) && closest == (length - 1))
            {
                precedingContext = [precedingContext stringByTrimmingCharactersInSet:
                                           [NSCharacterSet whitespaceAndNewlineCharacterSet]];
                length = precedingContext.length;
                if (includeWordSeps)
                {
                    // check to see if there is some sort of period or comma before the previous word
                    if (length > 0)
                    {
                        NSString *lastChar = [precedingContext substringWithRange:NSMakeRange(length - 1, 1)];
                        unichar last = [lastChar characterAtIndex:0];
                        if ([[DocumentUtil getWordSeparators] characterIsMember:last])
                        {
                            return @"";
                        }
                    }
                    else
                    {
                        return @"";
                    }
                }
            }
            
            spaceIndex = [DocumentUtil lastIndexOf:SPACE in:precedingContext];
            newLineIndex = [DocumentUtil lastIndexOf:NEW_LINE in:precedingContext];
            closest = spaceIndex > newLineIndex ? spaceIndex : newLineIndex;
            
            if ((spaceIndex > 0 || newLineIndex > 0))
            {
                precedingContext = [precedingContext substringFromIndex:(closest + 1)];
                return precedingContext;
            }
        }
    }
    
    return @"";
}

// Convert a unicode string into unescaped string (ie "\\u00e0" -> "à")
+ (NSString *)convertUnicodeToString:(NSString *)str
{
    NSString *convertedString = [str mutableCopy];
    CFStringRef transform = CFSTR("Any-Hex/Java");
    CFStringTransform((__bridge CFMutableStringRef)convertedString, NULL, transform, YES);
    
    return convertedString;
}

+ (bool)string:(NSString *)string containsSubstring:(NSString *)substring
{
    if (string == nil || substring == nil)
    {
        return NO;
    }
    
    if ([string rangeOfString:substring].location == NSNotFound)
    {
        return NO;
    }
    else
    {
        return YES; 
    }
}

@end

