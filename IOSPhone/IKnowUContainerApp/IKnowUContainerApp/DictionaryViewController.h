//
//  DictionaryViewController.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-10-01.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <Parse/Parse.h>

@interface DictionaryViewController : UIViewController <UITableViewDataSource, UITableViewDelegate>

@property (strong, nonatomic) IBOutlet UITableView *dictionaryTable;

- (void)addWord:(NSString *)word;

@end
