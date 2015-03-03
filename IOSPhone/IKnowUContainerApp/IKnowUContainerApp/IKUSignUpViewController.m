//
//  IKUSignUpViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-12-03.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "IKUSignUpViewController.h"

@interface IKUSignUpViewController ()

@end

@implementation IKUSignUpViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");
    
    UILabel *label = [[UILabel alloc] init];
    label.text = @"Sign up for Cloud Sync";
    label.textColor = [UIColor darkGrayColor];
    [label setFont:[UIFont systemFontOfSize:26]];
    self.signUpView.logo = label;
}

- (void)viewDidLayoutSubviews
{
    [super viewDidLayoutSubviews];    
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
