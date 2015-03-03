//
//  KeyboardFeedbackViewController.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-11-04.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface KeyboardFeedbackViewController : UIViewController <UITableViewDataSource, UITableViewDelegate>

@property (strong, nonatomic) IBOutlet UITableView *keyboardFeedbackTableView;

@end
