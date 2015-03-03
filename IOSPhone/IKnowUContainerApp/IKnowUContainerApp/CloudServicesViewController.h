//
//  CloudServicesViewController.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-12-02.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <Parse/Parse.h>

@interface CloudServicesViewController : UIViewController <UITableViewDataSource, UITableViewDelegate, PFLogInViewControllerDelegate, PFSignUpViewControllerDelegate>

@property (strong, nonatomic) IBOutlet UITableView *cloudServicesTableView;

@end
