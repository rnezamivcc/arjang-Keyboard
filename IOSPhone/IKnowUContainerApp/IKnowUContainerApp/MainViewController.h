//
//  MainViewController.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-08.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface MainViewController : UIViewController <UITableViewDataSource, UITableViewDelegate>

@property (strong, nonatomic) IBOutlet UITableView *mainTableView;

@end

