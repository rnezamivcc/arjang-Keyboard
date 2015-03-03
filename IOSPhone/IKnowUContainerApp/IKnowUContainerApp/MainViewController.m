//
//  MainViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-08.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "SharedDefaults.h"
#import "MainViewController.h"
@import IKnowUCommonFramework;

#define SHOWDETAIL @"ShowDetail"
#define SETTINGS @"Settings"
#define LOCATION @"Location"
#define DICTIONARY @"Dictionary"
#define LEGAL @"Legal"

@interface MainViewController ()

@property (strong, nonatomic) NSMutableArray *sections;

@end

@implementation MainViewController
            
- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");

    self.sections = [[NSMutableArray alloc] init];
    
    // Add more sections and labels here - define up top too
    NSArray *labels = @[SETTINGS, LOCATION, DICTIONARY, LEGAL];
    [self.sections addObject:labels];
    
    self.mainTableView.dataSource = self;
    self.mainTableView.delegate = self;
    
    NSUserDefaults *userDefaults = [[NSUserDefaults alloc] initWithSuiteName:@"group.wordlogic.IKnowUContainerApp"];
    [SharedDefaults registerDefaultSettings:userDefaults];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    [self.mainTableView deselectRowAtIndexPath:[self.mainTableView indexPathForSelectedRow] animated:YES];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    // On ipad, show first item in master list as selected if nothing is currently selected
    if ([(NSString*)[UIDevice currentDevice].model hasPrefix:@"iPad"])
    {
        NSIndexPath *indexPath = [self.mainTableView indexPathForSelectedRow];
        if (indexPath == nil)
        {
            indexPath = [NSIndexPath indexPathForRow:0 inSection:0];
            [self.mainTableView selectRowAtIndexPath:indexPath animated:NO scrollPosition:UITableViewScrollPositionNone];
        }
    }
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
    
    if ([selectedLabel isEqualToString:SETTINGS])
    {
        UIViewController *view = [[UIStoryboard storyboardWithName:@"Main" bundle:nil] instantiateViewControllerWithIdentifier:@"SettingsNavController"];
        [self showDetailViewController:view sender:self];
    }
    else if ([selectedLabel isEqualToString:LOCATION])
    {
        UIViewController *view = [[UIStoryboard storyboardWithName:@"Main" bundle:nil] instantiateViewControllerWithIdentifier:@"LocationNavController"];        
        [self showDetailViewController:view sender:self];
    }    
    else if ([selectedLabel isEqualToString:DICTIONARY])
    {
        UIViewController *view = [[UIStoryboard storyboardWithName:@"Main" bundle:nil] instantiateViewControllerWithIdentifier:@"DictionaryNavController"];
        [self showDetailViewController:view sender:self];
    }
    else if ([selectedLabel isEqualToString:LEGAL])
    {
        UIViewController *view = [[UIStoryboard storyboardWithName:@"Main" bundle:nil] instantiateViewControllerWithIdentifier:@"LegalNavController"];
        [self showDetailViewController:view sender:self];
    }
}

@end
