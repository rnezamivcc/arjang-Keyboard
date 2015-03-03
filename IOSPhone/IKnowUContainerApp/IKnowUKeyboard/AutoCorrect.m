//
//  AutoCorrect.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-11-12.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "KeyPosition.h"
#import "AutoCorrect.h"

@implementation AutoCorrect

- (MYWCHAR *)convertNSStringToMyWChar:(NSString *)str
{
    NSInteger length = [str length];
    MYWCHAR *wBuffer = malloc(sizeof(MYWCHAR) * (length + 1));
    if (wBuffer)
    {
        wBuffer[0] = (WCHAR)NULL;
        const char *charBuf = [str UTF8String];
        int i = 0;
        for (; i < length; i++)
        {
            wBuffer[i] = charBuf[i];
        }
        wBuffer[i] = (WCHAR)NULL;
    }
    return wBuffer;
}

- (void)replaceBuffers:(NSString *)beforeCursor afterCursor:(NSString *)afterCursor
{
    MYWCHAR *bc = [self convertNSStringToMyWChar:beforeCursor];
    MYWCHAR *ac = [self convertNSStringToMyWChar:afterCursor];
    ReplaceBuffers(bc, ac);
    free(bc);
    free(ac);
}

- (void)reverseCorrection:(int)si autocorrect:(NSString *)autocorrect rep:(NSString *)rep
{
    MYWCHAR *ac = [self convertNSStringToMyWChar:autocorrect];
    MYWCHAR *r = [self convertNSStringToMyWChar:rep];
    ReverseCorrection(si, ac, r);
    free(ac);
    free(r);
}

- (void)loadKeyboardLayout:(NSArray *)keypos
{
    NSInteger size = [keypos count];
    if (size > 0)
    {
        KeyPositionIOS *keys = malloc(sizeof(KeyPositionIOS) * size);
        NSInteger i = 0;
        for (id object in keypos)
        {
            // do something with object
            if ([object isKindOfClass:[KeyPosition class]])
            {
                KeyPosition *keyPosition = (KeyPosition *)object;
                keys[i].ch = keyPosition.ch;
                keys[i].x1 = keyPosition.x1;
                keys[i].y1 = keyPosition.y1;
                keys[i].x2 = keyPosition.x2;
                keys[i].y2 = keyPosition.y2;
                i++;
            }            
        }
        LoadKeyboardLayout(keys);
        free(keys);
    }
}

@end
