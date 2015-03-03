//
//  AssistanceViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-10-14.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "IKUSwitch.h"
#import "SharedDefaults.h"
#import "AssistanceViewController.h"

#define AUTO_CAPITALIZE @"Auto-Capitalize"
#define AUTO_LEARN @"Auto-Learn"
#define AUTO_CORRECT @"Auto-Correct"
#define AUTO_CORRECT_INSERT @"Auto-Correct Insert"
#define DOUBLE_SPACE @"Double Space Insert"

#define AUTO_CAPITALIZE_ROW 0
#define AUTO_LEARN_ROW 0
#define AUTO_CORRECT_ROW 0
#define AUTO_CORRECT_INSERT_ROW 0
#define DOUBLE_SPACE_ROW 0

#define AUTO_CAPITALIZE_DESCRIPTION @"Currently words that start sentences will be automatically capitalized"
#define AUTO_LEARN_DESCRIPTION @"Words with a '+' sign beside them will be learned when tapped"
#define AUTO_CORRECT_DESCRIPTION @"Currently IKnowU will be showing corrections in the prediction bar"
#define AUTO_CORRECT_INSERT_DESCRIPTION @"Currently auto-correct will automatically insert a correction for you"
#define DOUBLE_SPACE_DESCRIPTION @"Currently a period will be added if there are two consecutive spaces"

@interface AssistanceViewController ()

@property (strong, nonatomic) NSMutableArray *sections;
@property (strong, nonatomic) NSMutableArray *labelDescription;
@property (strong, nonatomic) NSMutableDictionary *labelKey;
@property (strong, nonatomic) NSUserDefaults *userDefaults;

@end

@implementation AssistanceViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");
    
    self.sections = [[NSMutableArray alloc] init];
    
    // Add more sections and labels here - define up top too
    NSArray *labels = @[AUTO_CAPITALIZE];
    [self.sections addObject:labels];
    
    labels = @[AUTO_LEARN];
    [self.sections addObject:labels];
    
    labels = @[AUTO_CORRECT];
    [self.sections addObject:labels];
    
    labels = @[AUTO_CORRECT_INSERT];
    [self.sections addObject:labels];
    
    labels = @[DOUBLE_SPACE];
    [self.sections addObject:labels];
    
    self.labelDescription = [[NSMutableArray alloc] init];
    
    [self.labelDescription addObject:AUTO_CAPITALIZE_DESCRIPTION];
    [self.labelDescription addObject:AUTO_LEARN_DESCRIPTION];
    [self.labelDescription addObject:AUTO_CORRECT_DESCRIPTION];
    [self.labelDescription addObject:AUTO_CORRECT_INSERT_DESCRIPTION];
    [self.labelDescription addObject:DOUBLE_SPACE_DESCRIPTION];
    
    self.labelKey = [[NSMutableDictionary alloc] init];
    
    [self.labelKey setObject:AUTO_CAPITALIZE_PREFERENCE forKey:AUTO_CAPITALIZE];
    [self.labelKey setObject:AUTO_LEARN_PREFERENCE forKey:AUTO_LEARN];
    [self.labelKey setObject:AUTO_CORRECT_PREFERENCE forKey:AUTO_CORRECT];
    [self.labelKey setObject:AUTO_CORRECT_INSERT_PREFERENCE forKey:AUTO_CORRECT_INSERT];
    [self.labelKey setObject:DOUBLE_SPACE_PREFERENCE forKey:DOUBLE_SPACE];
    
    self.assistanceTableView.dataSource = self;
    self.assistanceTableView.delegate = self;
    
    self.userDefaults = [[NSUserDefaults alloc] initWithSuiteName:@"group.wordlogic.IKnowUContainerApp"];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    [self.assistanceTableView deselectRowAtIndexPath:[self.assistanceTableView indexPathForSelectedRow] animated:YES];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void) switchChanged:(id)sender
{
    IKUSwitch* switchControl = sender;
    NSString *selectedLabel = switchControl.labelId;
    
    [self.userDefaults setBool:switchControl.on forKey:[self.labelKey objectForKey:selectedLabel]];
    [self.userDefaults synchronize];
}

#pragma mark - Table View DataSource

- (NSString *)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section
{
    return [self.labelDescription objectAtIndex:section];
}

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
    // NSIndexPath contains an array of indexes. For UITableView:
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
    NSUInteger sectionNumber = [indexPath indexAtPosition:0];
    NSUInteger rowNumber = [indexPath indexAtPosition:1];
    cell.textLabel.text = [[self.sections objectAtIndex:sectionNumber] objectAtIndex:rowNumber];
    
    cell.selectionStyle = UITableViewCellSelectionStyleNone;
    IKUSwitch *switchView = [[IKUSwitch alloc] initWithFrame:CGRectZero];
    [switchView setLabelId:cell.textLabel.text];
    cell.accessoryView = switchView;
    
    BOOL switchState = NO;
    switchState = [self.userDefaults boolForKey:[self.labelKey objectForKey:switchView.labelId]];
    [switchView setOn:switchState animated:NO];
    [switchView addTarget:self action:@selector(switchChanged:) forControlEvents:UIControlEventValueChanged];
    
    return cell;
}

#pragma mark - Table View Delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Get the selected cell
    UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
    [cell setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
    
    //
}

@end
