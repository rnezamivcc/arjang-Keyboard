//
//  SettingsViewController.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-24.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface SettingsViewController : UIViewController <UITableViewDataSource, UITableViewDelegate>

@property (strong, nonatomic) IBOutlet UITableView *settingsTableView;

@end
