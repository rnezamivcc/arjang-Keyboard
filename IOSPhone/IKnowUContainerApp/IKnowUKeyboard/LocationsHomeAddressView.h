//
//  LocationsHomeAddressView.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-30.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "LocationsButton.h"

@protocol LocationsHomeAddressViewDelegate

- (void)backHomeAddressAction;
- (void)doneHomeAddressAction:(NSString *)s;
- (void)clipHomeAddressAction:(NSString *)s;

@end

@interface LocationsHomeAddressView : UIView

@property (weak) id <LocationsHomeAddressViewDelegate> delegate;
@property (strong, nonatomic) NSMutableString *s;
@property (strong, nonatomic) NSUserDefaults *userDefaults;

@property (strong, nonatomic) NSString *apartmentNumber;
@property (strong, nonatomic) NSString *street;
@property (strong, nonatomic) NSString *city;
@property (strong, nonatomic) NSString *province;
@property (strong, nonatomic) NSString *country;
@property (strong, nonatomic) NSString *postalCode;

@property (strong, nonatomic) IBOutlet LocationsButton *backButton;
@property (strong, nonatomic) IBOutlet LocationsButton *doneButton;
@property (strong, nonatomic) IBOutlet LocationsButton *clipButton;

@property (strong, nonatomic) IBOutlet LocationsButton *aptNumButton;
@property (strong, nonatomic) IBOutlet LocationsButton *streetButton;
@property (strong, nonatomic) IBOutlet LocationsButton *cityButton;
@property (strong, nonatomic) IBOutlet LocationsButton *provinceButton;
@property (strong, nonatomic) IBOutlet LocationsButton *countryButton;

- (void)clear;
- (void)setupDetails;

- (IBAction)backPressed:(id)sender;
- (IBAction)donePressed:(id)sender;
- (IBAction)clipPressed:(id)sender;
- (IBAction)otherPressed:(id)sender;

@end
