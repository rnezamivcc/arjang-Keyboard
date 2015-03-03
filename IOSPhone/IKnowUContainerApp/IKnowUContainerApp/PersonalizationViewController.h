//
//  PersonalizationViewController.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-24.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "TimeoutViewController.h"

@interface PersonalizationViewController : UIViewController <UITableViewDataSource, UITableViewDelegate, TimeoutViewDelegate>

@property (strong, nonatomic) IBOutlet UITableView *personalizationTableView;

@end
