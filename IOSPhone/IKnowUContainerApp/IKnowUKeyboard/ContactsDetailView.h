//
//  ContactsDetailView.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-07.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Contact.h"

@protocol ContactsDetailViewDelegate

- (void)backAction;
- (void)doneAction:(NSString *)s;
- (void)clipAction:(NSString *)s;

@end

@interface ContactsDetailView : UIView

@property (weak) id <ContactsDetailViewDelegate> delegate;
@property (strong, nonatomic) Contact *contact;
@property (strong, nonatomic) NSMutableString *s;

@property (strong, nonatomic) IBOutlet UIImageView *contactImageView;

@property (strong, nonatomic) IBOutlet UIButton *backButton;
@property (strong, nonatomic) IBOutlet UIButton *doneButton;
@property (strong, nonatomic) IBOutlet UIButton *clipButton;

@property (strong, nonatomic) IBOutlet UIButton *nameButton;
@property (strong, nonatomic) IBOutlet UIButton *lastNameButton;
@property (strong, nonatomic) IBOutlet UIButton *aButton;
@property (strong, nonatomic) IBOutlet UIButton *bButton;
@property (strong, nonatomic) IBOutlet UIButton *cButton;
@property (strong, nonatomic) IBOutlet UIButton *dButton;

@property (strong, nonatomic) IBOutlet UILabel *aLabel;
@property (strong, nonatomic) IBOutlet UILabel *bLabel;
@property (strong, nonatomic) IBOutlet UILabel *cLabel;
@property (strong, nonatomic) IBOutlet UILabel *dLabel;

- (void)clear;
- (void)setupDetails:(Contact *)contact defaultImage:(UIImage *)defaultImage;

- (IBAction)backPressed:(id)sender;
- (IBAction)donePressed:(id)sender;
- (IBAction)clipPressed:(id)sender;
- (IBAction)otherPressed:(id)sender;

@end
