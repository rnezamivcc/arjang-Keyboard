//
//  LocationsSearchDetailView.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-09.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "LocationsButton.h"
#import "LocationsSearchResult.h"

@protocol LocationsSearchDetailViewDelegate

- (void)backSearchAction;
- (void)doneSearchAction:(NSString *)s;
- (void)clipSearchAction:(NSString *)s;

@end

@interface LocationsSearchDetailView : UIView

@property (weak) id <LocationsSearchDetailViewDelegate> delegate;
@property (strong, nonatomic) LocationsSearchResult *result;
@property (strong, nonatomic) NSMutableString *s;

@property (strong, nonatomic) IBOutlet LocationsButton *backButton;
@property (strong, nonatomic) IBOutlet LocationsButton *doneButton;
@property (strong, nonatomic) IBOutlet LocationsButton *clipButton;

@property (strong, nonatomic) IBOutlet LocationsButton *nameButton;
@property (strong, nonatomic) IBOutlet LocationsButton *streetButton;
@property (strong, nonatomic) IBOutlet LocationsButton *cityButton;
@property (strong, nonatomic) IBOutlet LocationsButton *provinceButton;
@property (strong, nonatomic) IBOutlet LocationsButton *countryButton;

- (void)clear;
- (void)setupDetails:(LocationsSearchResult *)result;

- (IBAction)backPressed:(id)sender;
- (IBAction)donePressed:(id)sender;
- (IBAction)clipPressed:(id)sender;
- (IBAction)otherPressed:(id)sender;

@end
