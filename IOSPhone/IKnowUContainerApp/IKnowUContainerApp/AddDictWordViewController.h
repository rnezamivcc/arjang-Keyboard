//
//  AddDictWordViewController.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-10-01.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface AddDictWordViewController : UIViewController <UITableViewDataSource, UITableViewDelegate, UITextFieldDelegate>

@property (strong, nonatomic) IBOutlet UITableView *tableView;

@end
