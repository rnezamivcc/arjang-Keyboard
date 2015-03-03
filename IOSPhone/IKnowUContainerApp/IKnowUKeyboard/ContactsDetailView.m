//
//  ContactsDetailView.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-07.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "ContactsDetailView.h"

@implementation ContactsDetailView

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
    [self.lastNameButton setTitle:@"" forState:UIControlStateNormal];
    [self.aButton setTitle:@"" forState:UIControlStateNormal];
    [self.bButton setTitle:@"" forState:UIControlStateNormal];
    [self.cButton setTitle:@"" forState:UIControlStateNormal];
    [self.dButton setTitle:@"" forState:UIControlStateNormal];
    
    // 1 = unselected, 2 = selected
    self.nameButton.tag = 1;
    self.lastNameButton.tag = 1;
    self.aButton.tag = 1;
    self.bButton.tag = 1;
    self.cButton.tag = 1;
    self.dButton.tag = 1;
    
    [self.nameButton setBackgroundColor:RGB(0.0, 122.0, 255.0)];
    [self.lastNameButton setBackgroundColor:RGB(0.0, 122.0, 255.0)];
    [self.aButton setBackgroundColor:RGB(0.0, 122.0, 255.0)];
    [self.bButton setBackgroundColor:RGB(0.0, 122.0, 255.0)];
    [self.cButton setBackgroundColor:RGB(0.0, 122.0, 255.0)];
    [self.dButton setBackgroundColor:RGB(0.0, 122.0, 255.0)];
    
    self.aLabel.text = @"";
    self.bLabel.text = @"";
    self.cLabel.text = @"";
    self.dLabel.text = @"";
    
    self.aButton.hidden = YES;
    self.bButton.hidden = YES;
    self.cButton.hidden = YES;
    self.dButton.hidden = YES;
    
    self.aLabel.hidden = YES;
    self.bLabel.hidden = YES;
    self.cLabel.hidden = YES;
    self.dLabel.hidden = YES;
}

- (void)setupDetails:(Contact *)contact defaultImage:(UIImage *)defaultImage
{    
    if (contact != nil)
    {
        self.contact = contact;
        
        UIImage *img = [contact img];
        if (img != nil)
        {
            self.contactImageView.image = img;
        }
        else
        {
            self.contactImageView.image = defaultImage;
        }
        CGRect frame = self.contactImageView.frame;
        CALayer *imageLayer = self.contactImageView.layer;
        [imageLayer setCornerRadius:frame.size.width/2];
        [imageLayer setBorderWidth:0];
        [imageLayer setMasksToBounds:YES];
        
        [self.nameButton setTitle:contact.firstName forState:UIControlStateNormal];
        [self.lastNameButton setTitle:contact.lastName forState:UIControlStateNormal];
        
        NSString *email = ([[contact email] count] > 0) ? [[contact email] objectAtIndex:0] : nil;
        NSUInteger phoneCount = MIN(3, [[contact phoneNumber] count]);
        if (email != nil)
        {
            self.aButton.hidden = NO;
            [self.aButton setTitle:email forState:UIControlStateNormal];
            NSString *emailLabel = ([[contact emailLabel] count] > 0) ? [[contact emailLabel] objectAtIndex:0] : nil;
            if (emailLabel != nil)
            {
                self.aLabel.hidden = NO;
                [self.aLabel setText:[emailLabel uppercaseString]];
            }
            
            switch (phoneCount)
            {
                case 3:
                {
                    self.dButton.hidden = NO;
                    self.dLabel.hidden = NO;
                    [self.dButton setTitle:[[contact phoneNumber] objectAtIndex:2] forState:UIControlStateNormal];
                    [self.dLabel setText:[[[contact phoneNumberLabel] objectAtIndex:2] uppercaseString]];
                }
                    
                case 2:
                {
                    self.cButton.hidden = NO;
                    self.cLabel.hidden = NO;
                    [self.cButton setTitle:[[contact phoneNumber] objectAtIndex:1] forState:UIControlStateNormal];
                    [self.cLabel setText:[[[contact phoneNumberLabel] objectAtIndex:1] uppercaseString]];
                }
                    
                case 1:
                {
                    self.bButton.hidden = NO;
                    self.bLabel.hidden = NO;
                    [self.bButton setTitle:[[contact phoneNumber] objectAtIndex:0] forState:UIControlStateNormal];
                    [self.bLabel setText:[[[contact phoneNumberLabel] objectAtIndex:0] uppercaseString]];
                    break;
                }
            }
        }
        else
        {
            switch (phoneCount)
            {
                case 3:
                {
                    self.cButton.hidden = NO;
                    self.cLabel.hidden = NO;
                    [self.cButton setTitle:[[contact phoneNumber] objectAtIndex:2] forState:UIControlStateNormal];
                    [self.cLabel setText:[[[contact phoneNumberLabel] objectAtIndex:2] uppercaseString]];
                }
                    
                case 2:
                {
                    self.bButton.hidden = NO;
                    self.bLabel.hidden = NO;
                    [self.bButton setTitle:[[contact phoneNumber] objectAtIndex:1] forState:UIControlStateNormal];
                    [self.bLabel setText:[[[contact phoneNumberLabel] objectAtIndex:1] uppercaseString]];
                }
                    
                case 1:
                {
                    self.aButton.hidden = NO;
                    self.aLabel.hidden = NO;
                    [self.aButton setTitle:[[contact phoneNumber] objectAtIndex:0] forState:UIControlStateNormal];
                    [self.aLabel setText:[[[contact phoneNumberLabel] objectAtIndex:0] uppercaseString]];
                    break;
                }
            }
        }
    }    
}

// return a formatted string based on selected buttons
- (NSString *)determineText
{
    [self.s setString:@""];
    if ((self.nameButton.tag == 2) && [self.contact.firstName length])
    {
        [self.s appendFormat:@" %@", self.contact.firstName];
    }
    
    if ((self.lastNameButton.tag == 2) && [self.contact.lastName length])
    {
        if ([self.s length] > 0)
        {
            [self.s appendFormat:@" %@", self.contact.lastName];
        }
        else
        {
            [self.s appendFormat:@"%@", self.contact.lastName];
        }
    }
    
    if (self.aButton.tag == 2)
    {
        [self.s appendFormat:@" at %@", self.aButton.titleLabel.text];
    }
    
    if (self.bButton.tag == 2)
    {
        if ([self.s length] > 0)
        {
            [self.s appendFormat:@" or %@", self.bButton.titleLabel.text];
        }
        else
        {
            [self.s appendFormat:@" at %@", self.bButton.titleLabel.text];
        }
    }
    
    if (self.cButton.tag == 2)
    {
        if ([self.s length] > 0)
        {
            [self.s appendFormat:@" or %@", self.cButton.titleLabel.text];
        }
        else
        {
            [self.s appendFormat:@" at %@", self.cButton.titleLabel.text];
        }
    }
    
    if (self.dButton.tag == 2)
    {
        if ([self.s length] > 0)
        {
            [self.s appendFormat:@" or %@", self.dButton.titleLabel.text];
        }
        else
        {
            [self.s appendFormat:@" at %@", self.dButton.titleLabel.text];
        }
    }
    
    return self.s;
}

- (IBAction)backPressed:(id)sender
{
    [self.delegate backAction];
}

- (IBAction)donePressed:(id)sender
{
    [self.delegate doneAction:[self determineText]];
}

- (IBAction)clipPressed:(id)sender
{
    [self.delegate clipAction:[self determineText]];
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
