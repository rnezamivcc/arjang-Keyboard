//
//  SettingsViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-24.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "SettingsViewController.h"

#define ASSISTANCE @"Assistance"
#define KEYBOARD_FEEDBACK @"Keyboard Feedback"
#define CLOUD_SERVICES @"Cloud Services"
#define PERSONALIZATION @"Personalization"

@interface SettingsViewController ()

@property (strong, nonatomic) NSMutableArray *sections;

@end

@implementation SettingsViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");
    
    self.sections = [[NSMutableArray alloc] init];
    
    // Add more sections and labels here - define up top too
    NSArray *labels = @[ASSISTANCE, KEYBOARD_FEEDBACK, CLOUD_SERVICES, PERSONALIZATION];
    [self.sections addObject:labels];
    
    self.settingsTableView.dataSource = self;
    self.settingsTableView.delegate = self;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    [self.settingsTableView deselectRowAtIndexPath:[self.settingsTableView indexPathForSelectedRow] animated:YES];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table View DataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Number of sections
    return [self.sections count];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Number of rows
    return [[self.sections objectAtIndex:section] count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    // NSIndexPath contains an array of indexes.  For UITableView:
    //    indexAtPosition:0 is the section number
    //    indexAtPosition:1 is the row number
    
    // Create an identifier for this type of cell
    static NSString *CellIdentifier = @"Cell";
    
    // Get a cell of this type from the re-use queue or create one
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil)
    {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier];
    }
    
    // Configure the cell...
    cell.textLabel.text = [[self.sections objectAtIndex:[indexPath indexAtPosition:0]]
                           objectAtIndex:[indexPath indexAtPosition:1]];
    [cell setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
    
    return cell;
}

#pragma mark - Table View Delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Get the selected cell
    UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
    [cell setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
    
    NSString *selectedLabel = cell.textLabel.text;
    
    if ([selectedLabel isEqualToString:ASSISTANCE])
    {
        [self performSegueWithIdentifier:ASSISTANCE sender:self];
    }
    
    else if ([selectedLabel isEqualToString:KEYBOARD_FEEDBACK])
    {
        [self performSegueWithIdentifier:KEYBOARD_FEEDBACK sender:self];
    }
    
    else if ([selectedLabel isEqualToString:CLOUD_SERVICES])
    {
        [self performSegueWithIdentifier:CLOUD_SERVICES sender:self];
    }
    
    else if ([selectedLabel isEqualToString:PERSONALIZATION])
    {
        [self performSegueWithIdentifier:PERSONALIZATION sender:self];
    }
}

@end
