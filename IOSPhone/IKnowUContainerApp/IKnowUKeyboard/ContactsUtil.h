//
//  ContactsUtil.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-07.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ContactsUtil : NSObject

+ (NSArray *)loadContacts:(NSString *)searchBy;

@end
