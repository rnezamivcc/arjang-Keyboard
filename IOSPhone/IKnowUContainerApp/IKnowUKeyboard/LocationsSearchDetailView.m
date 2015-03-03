//
//  LocationsSearchDetailView.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-09.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "LocationsSearchDetailView.h"

@implementation LocationsSearchDetailView

- (id)initWithCoder:(NSCoder*)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self)
    {
        self.s = [[NSMutableString alloc] init];
    }
    return self;
}

- (void)clear
{
    [self.nameButton setTitle:@"" forState:UIControlStateNormal];
    [self.streetButton setTitle:@"" forState:UIControlStateNormal];
    [self.cityButton setTitle:@"" forState:UIControlStateNormal];
    [self.provinceButton setTitle:@"" forState:UIControlStateNormal];
    [self.countryButton setTitle:@"" forState:UIControlStateNormal];
    
    // 1 = unselected, 2 = selected
    self.nameButton.tag = 1;
    self.streetButton.tag = 1;
    self.cityButton.tag = 1;
    self.provinceButton.tag = 1;
    self.countryButton.tag = 1;
    
    [self.nameButton setBackgroundColor:RGB(0.0, 122.0, 255.0)];
    [self.streetButton setBackgroundColor:RGB(0.0, 122.0, 255.0)];
    [self.cityButton setBackgroundColor:RGB(0.0, 122.0, 255.0)];
    [self.provinceButton setBackgroundColor:RGB(0.0, 122.0, 255.0)];
    [self.countryButton setBackgroundColor:RGB(0.0, 122.0, 255.0)];
}

- (void)setupDetails:(LocationsSearchResult *)result
{
    if (result != nil)
    {
        self.result = result;
        
        [self.nameButton setTitle:result.name forState:UIControlStateNormal];
        [self.streetButton setTitle:result.street forState:UIControlStateNormal];
        [self.cityButton setTitle:result.city forState:UIControlStateNormal];
        [self.provinceButton setTitle:result.province forState:UIControlStateNormal];
        [self.countryButton setTitle:result.country forState:UIControlStateNormal];
    }
}

// return a formatted string based on selected buttons
- (NSString *)determineText
{
    [self.s setString:@""];
    if (self.nameButton.tag == 2)
    {
        [self.s appendFormat:@" %@", self.result.name];
    }
    
    if (self.streetButton.tag == 2)
    {
        if ([self.s length] > 0)
        {
            [self.s appendString:@" located at "];
        }
        [self.s appendFormat:@" %@", self.result.street];
    }
    
    if (self.cityButton.tag == 2)
    {
        if ([self.s length] > 0)
        {
            [self.s appendString:@", "];
        }
        [self.s appendFormat:@" %@", self.result.city];
    }
    
    if (self.provinceButton.tag == 2)
    {
        if ([self.s length] > 0)
        {
            [self.s appendString:@", "];
        }
        [self.s appendFormat:@" %@", self.result.province];
    }
    
    if (self.countryButton.tag == 2)
    {
        if ([self.s length] > 0)
        {
            [self.s appendString:@", "];
        }
        [self.s appendFormat:@" %@", self.result.country];
    }
    
    return self.s;
}

- (IBAction)backPressed:(id)sender
{
    [self.delegate backSearchAction];
}

- (IBAction)donePressed:(id)sender
{
    [self.delegate doneSearchAction:[self determineText]];
}

- (IBAction)clipPressed:(id)sender
{
    [self.delegate clipSearchAction:[self determineText]];
}

- (IBAction)otherPressed:(id)sender
{
    UIButton *button = (UIButton *)sender;
    if (button.tag == 1)
    {
        [button setBackgroundColor:RGB(0.0, 122.0, 0.0)];
        button.tag = 2;
    }
    else if (button.tag == 2)
    {
        [button setBackgroundColor:RGB(0.0, 122.0, 255.0)];
        button.tag = 1;
    }
}

@end
