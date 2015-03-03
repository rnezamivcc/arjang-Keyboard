//
//  AssistanceViewController.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-10-14.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface AssistanceViewController : UIViewController <UITableViewDataSource, UITableViewDelegate>

@property (strong, nonatomic) IBOutlet UITableView *assistanceTableView;

@end
