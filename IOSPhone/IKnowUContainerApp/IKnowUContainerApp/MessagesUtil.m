//
//  MessagesUtil.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-12-15.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "MessagesUtil.h"

@implementation MessagesUtil

+ (UIAlertController *)showTitleMessageOK:(NSString *)title message:(NSString*)message
{
    __block UIAlertController *alert = [UIAlertController alertControllerWithTitle:title
                                                                           message:message
                                                                    preferredStyle:UIAlertControllerStyleAlert];
    
    UIAlertAction *ok = [UIAlertAction actionWithTitle:@"OK"
                                                 style:UIAlertActionStyleDefault
                                               handler:^(UIAlertAction *action)
                         {
                             [alert dismissViewControllerAnimated:YES completion:nil];
                         }];
    
    [alert addAction:ok];
    
    return alert;
}

@end
