//
//  TimeoutViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-11-05.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "TimeoutViewController.h"

@interface TimeoutViewController ()

@end

@implementation TimeoutViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");
    
    [self.navigationItem setTitle:self.suggestedTitle];
    [self.timeSlider setMinimumValue:self.minimumValue];
    [self.timeSlider setMaximumValue:self.maximumValue];
    [self.timeSlider setValue:self.value];
    
    [self.timeLabel setText:[NSString stringWithFormat:@"%3.0f ms", self.value]];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)didMoveToParentViewController:(UIViewController *)parent
{
    if (![parent isEqual:self.parentViewController])
    {
        if (self.delegate)
        {
            [self.delegate result:self.value title:self.suggestedTitle];
        }
    }
}

- (IBAction)timeSliderChanged:(id)sender
{
    UISlider *slider = (UISlider *)sender;
    
    self.value = slider.value;
    [self.timeLabel setText:[NSString stringWithFormat:@"%3.0f ms", self.value]];
}

@end
