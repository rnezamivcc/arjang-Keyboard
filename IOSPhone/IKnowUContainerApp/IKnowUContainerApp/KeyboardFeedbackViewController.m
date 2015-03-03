//
//  KeyboardFeedbackViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-11-04.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "IKUSwitch.h"
#import "SharedDefaults.h"
#import "KeyboardFeedbackViewController.h"

#define SOUND_ON_KEYPRESS @"Sound on Key Press"
#define HIGHLIGHTED_KEYS @"Highlighted Keys"
#define CORRECTION_INDICATION @"Correction Indication"
#define COMMENT @"Comment or Report Problem"

#define COMMENT_SECTION 3

#define SOUND_ON_KEYPRESS_ROW 0
#define HIGHLIGHTED_KEYS_ROW 0
#define CORRECTION_INDICATION_ROW 0
#define COMMENT_ROW 0

#define SOUND_ON_KEYPRESS_DESCRIPTION @"Enable sound when key is pressed"
#define HIGHLIGHTED_KEYS_DESCRIPTION @"Currently, the next most likely keys you will press, will be highlighted in color"
#define CORRECTION_INDICATION_DESCRIPTION @"Currently, when a correction is made, there will be no feedback from the keyboard"
#define COMMENT_DESCRIPTION @"Tell IKnowU what you think, we'd love to hear your input"

@interface KeyboardFeedbackViewController ()

@property (strong, nonatomic) NSMutableArray *sections;
@property (strong, nonatomic) NSMutableArray *labelDescription;
@property (strong, nonatomic) NSMutableDictionary *labelKey;
@property (strong, nonatomic) NSUserDefaults *userDefaults;

@end

@implementation KeyboardFeedbackViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");
    
    self.sections = [[NSMutableArray alloc] init];
    
    // Add more sections and labels here - define up top too
    NSArray *labels = @[SOUND_ON_KEYPRESS];
    [self.sections addObject:labels];
    
    labels = @[HIGHLIGHTED_KEYS];
    [self.sections addObject:labels];
    
    labels = @[CORRECTION_INDICATION];
    [self.sections addObject:labels];
    
    labels = @[COMMENT];
    [self.sections addObject:labels];
    
    self.labelDescription = [[NSMutableArray alloc] init];
    
    [self.labelDescription addObject:SOUND_ON_KEYPRESS_DESCRIPTION];
    [self.labelDescription addObject:HIGHLIGHTED_KEYS_DESCRIPTION];
    [self.labelDescription addObject:CORRECTION_INDICATION_DESCRIPTION];
    [self.labelDescription addObject:COMMENT_DESCRIPTION];
    
    self.labelKey = [[NSMutableDictionary alloc] init];
    
    [self.labelKey setObject:SOUND_ON_KEYPRESS_PREFERENCE forKey:SOUND_ON_KEYPRESS];
    [self.labelKey setObject:HIGHLIGHTED_KEYS_PREFERENCE forKey:HIGHLIGHTED_KEYS];
    [self.labelKey setObject:CORRECTION_INDICATION_PREFERENCE forKey:CORRECTION_INDICATION];
    
    self.keyboardFeedbackTableView.dataSource = self;
    self.keyboardFeedbackTableView.delegate = self;
    
    self.userDefaults = [[NSUserDefaults alloc] initWithSuiteName:@"group.wordlogic.IKnowUContainerApp"];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    [self clearSelection];
}

- (void)clearSelection
{
    [self.keyboardFeedbackTableView deselectRowAtIndexPath:[self.keyboardFeedbackTableView indexPathForSelectedRow] animated:YES];
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
    
    if (sectionNumber == 0 || sectionNumber == 1 || sectionNumber == 2)
    {
        cell.selectionStyle = UITableViewCellSelectionStyleNone;
        IKUSwitch *switchView = [[IKUSwitch alloc] initWithFrame:CGRectZero];
        [switchView setLabelId:cell.textLabel.text];
        cell.accessoryView = switchView;
    
        BOOL switchState = NO;
        switchState = [self.userDefaults boolForKey:[self.labelKey objectForKey:switchView.labelId]];
        [switchView setOn:switchState animated:NO];
        [switchView addTarget:self action:@selector(switchChanged:) forControlEvents:UIControlEventValueChanged];
    }
    if (sectionNumber == COMMENT_SECTION)
    {
        cell.textLabel.textColor = [UIColor blueColor];
        [cell setAccessoryType:UITableViewCellAccessoryNone];
    }
    
    return cell;
}

#pragma mark - Table View Delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSUInteger sectionNumber = [indexPath indexAtPosition:0];
    NSUInteger rowNumber = [indexPath indexAtPosition:1];
    
    if ((sectionNumber == COMMENT_SECTION) && (rowNumber == COMMENT_ROW))
    {
        [[UIApplication sharedApplication] openURL:[NSURL URLWithString: @"http://www.iknowu.net/support/"]];
        [self clearSelection];
    }
}

@end
