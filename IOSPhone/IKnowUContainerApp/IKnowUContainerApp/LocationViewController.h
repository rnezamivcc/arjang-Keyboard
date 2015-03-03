//
//  LocationViewController.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-26.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface LocationViewController : UIViewController <UITableViewDataSource, UITableViewDelegate, UITextFieldDelegate>

@property (strong, nonatomic) NSString *apartmentNumber;
@property (strong, nonatomic) NSString *address;
@property (strong, nonatomic) NSString *city;
@property (strong, nonatomic) NSString *provinceState;
@property (strong, nonatomic) NSString *country;
@property (strong, nonatomic) NSString *postalZipCode;

@property (strong, nonatomic) IBOutlet UITableView *locationTable;

@end
