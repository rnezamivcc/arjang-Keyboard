//
//  LegalViewController.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-11-04.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface LegalViewController : UIViewController <UITableViewDataSource, UITableViewDelegate>

@property (strong, nonatomic) IBOutlet UITableView *legalTableView;

@end
