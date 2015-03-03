//
//  KeyboardThemesViewController.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-10-07.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface KeyboardThemesViewController : UIViewController <UITableViewDataSource, UITableViewDelegate>

@property (strong, nonatomic) IBOutlet UITableView *keyboardThemesTable;

@end
