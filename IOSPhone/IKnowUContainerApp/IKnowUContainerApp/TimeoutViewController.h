//
//  TimeoutViewController.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-11-05.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol TimeoutViewDelegate

- (void)result:(CGFloat)result title:(NSString *)title;

@end

@interface TimeoutViewController : UITableViewController

@property (weak) id <TimeoutViewDelegate> delegate;

@property (strong, nonatomic) NSString *suggestedTitle;
@property (nonatomic) CGFloat value;
@property (nonatomic) CGFloat minimumValue;
@property (nonatomic) CGFloat maximumValue;

@property (strong, nonatomic) IBOutlet UILabel *timeLabel;
@property (strong, nonatomic) IBOutlet UISlider *timeSlider;

- (IBAction)timeSliderChanged:(id)sender;

@end
