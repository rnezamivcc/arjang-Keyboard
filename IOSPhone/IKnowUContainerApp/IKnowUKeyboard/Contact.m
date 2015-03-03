//
//  Contact.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-12.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "Contact.h"

@implementation Contact

- (id)init
{
    self = [super init];
    if (self)
    {
        self.firstName = [[NSMutableString alloc] init];
        self.lastName = [[NSMutableString alloc] init];
        self.phoneNumber = [[NSMutableArray alloc] init];
        self.email = [[NSMutableArray alloc] init];
        self.phoneNumberLabel = [[NSMutableArray alloc] init];
        self.emailLabel = [[NSMutableArray alloc] init];
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"obj = %@ firstName[%@] lastName[%@] phoneNumber[%@] phoneNumberLabel[%@] email[%@] emailLabel[%@] img[%@]",
            [super description], self.firstName, self.lastName, self.phoneNumber, self.phoneNumberLabel, self.email, self.emailLabel, self.img];
}

@end
