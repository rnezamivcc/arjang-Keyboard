//
//  AboutViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-11-04.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "AboutViewController.h"

@interface AboutViewController ()

@end

@implementation AboutViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");
    
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
    [formatter setDateFormat:@"yyyy"];
    NSString *yearString = [formatter stringFromDate:[NSDate date]];
        
    self.copyrightLabel.text = [NSString stringWithFormat:@"(©) 2011 - %@ WordLogic Corporation. All Rights Reserved.\nConfidential property of WordLogic Corporation, not for redistribution.\nThis software is protected by \nU.S. Patents 7293231, 7681124, 7716579, 7921361 and by European patents 1171813 and 1356368. Further patents pending.\nFor more information about WordLogic, visit us at www.wordlogic.com", yearString];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
