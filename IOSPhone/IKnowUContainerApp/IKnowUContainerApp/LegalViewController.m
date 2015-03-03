//
//  LegalViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-11-04.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "LegalViewController.h"

#define ABOUT_WORDLOGIC @"About WordLogic"
#define LEGAL_AND_PRIVACY_NOTICE @"Legal and Privacy Notice"

@interface LegalViewController ()

@property (strong, nonatomic) NSMutableArray *sections;

@end

@implementation LegalViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");
    
    self.sections = [[NSMutableArray alloc] init];
    
    // Add more sections and labels here - define up top too
    NSArray *labels = @[ABOUT_WORDLOGIC, LEGAL_AND_PRIVACY_NOTICE];
    [self.sections addObject:labels];
    
    self.legalTableView.dataSource = self;
    self.legalTableView.delegate = self;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    [self.legalTableView deselectRowAtIndexPath:[self.legalTableView indexPathForSelectedRow] animated:YES];
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
    
    NSUInteger sectionNumber = [indexPath indexAtPosition:0];
    NSUInteger rowNumber = [indexPath indexAtPosition:1];
    
    // Get a cell of this type from the re-use queue or create one
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil)
    {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:CellIdentifier];
    }
    
    // Configure the cell...
    cell.textLabel.text = [[self.sections objectAtIndex:sectionNumber] objectAtIndex:rowNumber];
    [cell setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
    
    if (rowNumber == 0)
    {
        NSString *appVersionString = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
        cell.detailTextLabel.text = appVersionString;
    }
    
    return cell;
}

#pragma mark - Table View Delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Get the selected cell
    UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
    [cell setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
    
    NSString *selectedLabel = cell.textLabel.text;
    
    if ([selectedLabel isEqualToString:ABOUT_WORDLOGIC])
    {
        [self performSegueWithIdentifier:ABOUT_WORDLOGIC sender:self];
    }
    
    else if ([selectedLabel isEqualToString:LEGAL_AND_PRIVACY_NOTICE])
    {
        [self performSegueWithIdentifier:LEGAL_AND_PRIVACY_NOTICE sender:self];
    }
}

@end
