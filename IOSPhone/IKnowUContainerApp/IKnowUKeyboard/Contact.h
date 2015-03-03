//
//  Contact.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-12.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface Contact : NSObject

@property (strong, nonatomic) NSMutableString *firstName;
@property (strong, nonatomic) NSMutableString *lastName;
@property (strong, nonatomic) NSMutableArray *phoneNumber;
@property (strong, nonatomic) NSMutableArray *email;
@property (strong, nonatomic) NSMutableArray *phoneNumberLabel;
@property (strong, nonatomic) NSMutableArray *emailLabel;
@property (strong, nonatomic) UIImage *img;

@end
