//
//  PersonalizationViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-24.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "IKUSwitch.h"
#import "SharedDefaults.h"
#import "TimeoutViewController.h"
#import "PersonalizationViewController.h"

#define SWIPE_TYPING @"Swipe Typing"
#define PHRASE_PREDICTION @"Phrase Prediction"
#define LONG_PRESS_TIMEOUT @"Long Press Timeout"
#define WORD_DELETE_TIMEOUT @"Word Delete Timeout"
#define KEYBOARD_THEME @"Keyboard Theme"
#define REACH @"Reach"

#define SWIPE_TYPING_SECTION 0
#define PHRASE_PREDICTION_SECTION 1
#define LONG_PRESS_SECTION 2
#define WORD_DELETE_SECTION 3
#define KEYBOARD_THEME_SECTION 4
#define REACH_SECTION 5

#define SWIPE_TYPING_ROW 0
#define PHRASE_PREDICTION_ROW 0
#define LONG_PRESS_ROW 0
#define WORD_DELETE_ROW 0
#define KEYBOARD_THEME_ROW 0
#define REACH_ROW 0

#define SWIPE_TYPING_DESCRIPTION @"Enable swiping"
#define PHRASE_PREDICTION_DESCRIPTION @"Show phrase predictions"
#define LONG_PRESS_DESCRIPTION @"Determines how long you must hold down before a press becomes a long press"
#define WORD_DELETE_DESCRIPTION @"Determines how long you must hold down before deleting the next word"
#define KEYBOARD_THEME_DESCRIPTION @"Set the theme for your keyboard"
#define REACH_DESCRIPTION @"Enable Reach for quick access to contact/location information in keyboard"

#define KEYBOARDTHEMES_SEGUE @"KeyboardThemes"
#define TIMEOUT_SEGUE @"Timeout"

@interface PersonalizationViewController ()

@property (strong, nonatomic) NSMutableArray *sections;
@property (strong, nonatomic) NSMutableArray *labelDescription;
@property (strong, nonatomic) NSMutableDictionary *labelKey;
@property (strong, nonatomic) NSUserDefaults *userDefaults;

@end

@implementation PersonalizationViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");
    
    self.sections = [[NSMutableArray alloc] init];
    
    // Add more sections and labels here - define up top too
    NSArray *labels = @[SWIPE_TYPING];
    [self.sections addObject:labels];
    
    labels = @[PHRASE_PREDICTION];
    [self.sections addObject:labels];
    
    labels = @[LONG_PRESS_TIMEOUT];
    [self.sections addObject:labels];
    
    labels = @[WORD_DELETE_TIMEOUT];
    [self.sections addObject:labels];
    
    labels = @[KEYBOARD_THEME];
    [self.sections addObject:labels];
    
    labels = @[REACH];
    [self.sections addObject:labels];
    
    self.labelDescription = [[NSMutableArray alloc] init];
    
    [self.labelDescription addObject:SWIPE_TYPING_DESCRIPTION];
    [self.labelDescription addObject:PHRASE_PREDICTION_DESCRIPTION];
    [self.labelDescription addObject:LONG_PRESS_DESCRIPTION];
    [self.labelDescription addObject:WORD_DELETE_DESCRIPTION];
    [self.labelDescription addObject:KEYBOARD_THEME_DESCRIPTION];
    [self.labelDescription addObject:REACH_DESCRIPTION];
    
    self.labelKey = [[NSMutableDictionary alloc] init];
    
    [self.labelKey setObject:SWIPE_TYPING_PREFERENCE forKey:SWIPE_TYPING];
    [self.labelKey setObject:PHRASE_PREDICTION_PREFERENCE forKey:PHRASE_PREDICTION];
    [self.labelKey setObject:LONG_PRESS_PREFERENCE forKey:LONG_PRESS_TIMEOUT];
    [self.labelKey setObject:WORD_DELETE_PREFERENCE forKey:WORD_DELETE_TIMEOUT];
    [self.labelKey setObject:REACH_PREFERENCE forKey:REACH];
    
    self.personalizationTableView.dataSource = self;
    self.personalizationTableView.delegate = self;
    
    self.userDefaults = [[NSUserDefaults alloc] initWithSuiteName:@"group.wordlogic.IKnowUContainerApp"];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    [self.personalizationTableView deselectRowAtIndexPath:[self.personalizationTableView indexPathForSelectedRow] animated:YES];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)switchChanged:(id)sender
{
    IKUSwitch* switchControl = sender;
    NSString *selectedLabel = switchControl.labelId;
    
    if ([selectedLabel isEqualToString:SWIPE_TYPING] ||
        [selectedLabel isEqualToString:PHRASE_PREDICTION] ||
        [selectedLabel isEqualToString:REACH])
    {
        [self.userDefaults setBool:switchControl.on forKey:[self.labelKey objectForKey:selectedLabel]];
        [self.userDefaults synchronize];
    }
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
    NSUInteger sectionNumber = [indexPath indexAtPosition:0];
    NSUInteger rowNumber = [indexPath indexAtPosition:1];
    NSString *text = [[self.sections objectAtIndex:sectionNumber] objectAtIndex:rowNumber];
    
    if ([text isEqualToString:KEYBOARD_THEME] ||
        [text isEqualToString:LONG_PRESS_TIMEOUT] ||
        [text isEqualToString:WORD_DELETE_TIMEOUT])
    {
        cell.textLabel.text = text;
        [cell setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
    }
    else if ([text isEqualToString:SWIPE_TYPING] ||
             [text isEqualToString:PHRASE_PREDICTION] ||
             [text isEqualToString:REACH])
    {
        cell.textLabel.text = text;
        cell.selectionStyle = UITableViewCellSelectionStyleNone;
        IKUSwitch *switchView = [[IKUSwitch alloc] initWithFrame:CGRectZero];
        [switchView setLabelId:cell.textLabel.text];
        cell.accessoryView = switchView;
        
        BOOL switchState = NO;
        switchState = [self.userDefaults boolForKey:[self.labelKey objectForKey:switchView.labelId]];
        [switchView setOn:switchState animated:NO];
        [switchView addTarget:self action:@selector(switchChanged:) forControlEvents:UIControlEventValueChanged];
    }
    
    return cell;
}

#pragma mark - Table View Delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Get the selected cell
    UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
    [cell setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
    
    NSUInteger sectionNumber = [indexPath indexAtPosition:0];
    NSUInteger rowNumber = [indexPath indexAtPosition:1];
    if ((sectionNumber == KEYBOARD_THEME_SECTION) && (rowNumber == KEYBOARD_THEME_ROW))
    {
        [self performSegueWithIdentifier:KEYBOARDTHEMES_SEGUE sender:self];
    }
    else if ((sectionNumber == LONG_PRESS_SECTION) && (rowNumber == LONG_PRESS_ROW))
    {
        [self performSegueWithIdentifier:TIMEOUT_SEGUE sender:self];
    }
    else if ((sectionNumber == WORD_DELETE_SECTION) && (rowNumber == WORD_DELETE_ROW))
    {
        [self performSegueWithIdentifier:TIMEOUT_SEGUE sender:self];
    }
}

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if ([[segue identifier] isEqualToString:TIMEOUT_SEGUE])
    {
        TimeoutViewController *vc = [segue destinationViewController];
        
        NSIndexPath *indexPath = [self.personalizationTableView indexPathForSelectedRow];
        NSUInteger sectionNumber = [indexPath indexAtPosition:0];
        NSUInteger rowNumber = [indexPath indexAtPosition:1];
        
        if ((sectionNumber == LONG_PRESS_SECTION) && (rowNumber == LONG_PRESS_ROW))
        {
            [vc setSuggestedTitle:LONG_PRESS_TIMEOUT];            
            [vc setMinimumValue:LONG_PRESS_TIMEOUT_MIN];
            [vc setMaximumValue:LONG_PRESS_TIMEOUT_MAX];
            
            CGFloat value = [self.userDefaults floatForKey:[self.labelKey objectForKey:LONG_PRESS_TIMEOUT]];
            [vc setValue:(((LONG_PRESS_TIMEOUT_MIN <= value) && (value <= LONG_PRESS_TIMEOUT_MAX)) ? value : LONG_PRESS_TIMEOUT_MIN)];
            [vc setDelegate:self];
        }
        else if ((sectionNumber == WORD_DELETE_SECTION) && (rowNumber == WORD_DELETE_ROW))
        {
            [vc setSuggestedTitle:WORD_DELETE_TIMEOUT];
            [vc setMinimumValue:DELETE_WORD_TIMEOUT_MIN];
            [vc setMaximumValue:DELETE_WORD_TIMEOUT_MAX];
            
            CGFloat value = [self.userDefaults floatForKey:[self.labelKey objectForKey:WORD_DELETE_TIMEOUT]];
            [vc setValue:(((DELETE_WORD_TIMEOUT_MIN <= value) && (value <= DELETE_WORD_TIMEOUT_MAX)) ? value : DELETE_WORD_TIMEOUT_MIN)];
            [vc setDelegate:self];
        }
    }
}

#pragma mark - Timeout View Delegate

- (void)result:(CGFloat)result title:(NSString *)title
{
    [self.userDefaults setFloat:result forKey:[self.labelKey objectForKey:title]];
    [self.userDefaults synchronize];
}

@end
