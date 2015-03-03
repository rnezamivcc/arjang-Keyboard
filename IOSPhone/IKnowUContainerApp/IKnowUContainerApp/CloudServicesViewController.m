//
//  CloudServicesViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-12-02.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKULoginViewController.h"
#import "IKUSignUpViewController.h"
#import "IKUMacros.h"
#import "IKUSwitch.h"
#import "SharedDefaults.h"
#import "CloudServicesViewController.h"

#define SIGNUP_CLOUDSYNC @"Sign up/Login for Cloud Sync"
#define CLOUDSYNC_ONOFF @"Cloud Sync On/Off"

#define SIGNUP_CLOUDSYNC_SECTION 0
#define CLOUDSYNC_ONOFF_SECTION 1

#define SIGNUP_CLOUDSYNC_ROW 0
#define CLOUDSYNC_ONOFF_ROW 0

#define SIGNUP_CLOUDSYNC_DESCRIPTION @"By enabling cloud services you will be able to sync your dictionaries across all your devices, meaning that new words learned here, will show up on your other devices"
#define CLOUDSYNC_ONOFF_DESCRIPTION @"Synchronize dictionaries across all your devices"

@interface CloudServicesViewController ()

@property (strong, nonatomic) NSMutableArray *sections;
@property (strong, nonatomic) NSMutableArray *labelDescription;
@property (strong, nonatomic) NSMutableDictionary *labelKey;
@property (strong, nonatomic) NSUserDefaults *userDefaults;

@end

@implementation CloudServicesViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");
    
    self.sections = [[NSMutableArray alloc] init];
    
    // Add more sections and labels here - define up top too
    NSArray *labels = @[SIGNUP_CLOUDSYNC];
    [self.sections addObject:labels];
    
    labels = @[CLOUDSYNC_ONOFF];
    [self.sections addObject:labels];
    
    self.labelDescription = [[NSMutableArray alloc] init];
    
    [self.labelDescription addObject:SIGNUP_CLOUDSYNC_DESCRIPTION];
    [self.labelDescription addObject:CLOUDSYNC_ONOFF_DESCRIPTION];
    
    self.labelKey = [[NSMutableDictionary alloc] init];
    
    [self.labelKey setObject:CLOUDSYNC_ONOFF_PREFERENCE forKey:CLOUDSYNC_ONOFF];
    
    self.cloudServicesTableView.dataSource = self;
    self.cloudServicesTableView.delegate = self;
    
    self.userDefaults = [[NSUserDefaults alloc] initWithSuiteName:@"group.wordlogic.IKnowUContainerApp"];
    
    PFUser *user = [PFUser currentUser];
    [self updatePFUserToUserDefaults:user];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    [self.cloudServicesTableView deselectRowAtIndexPath:[self.cloudServicesTableView indexPathForSelectedRow] animated:YES];
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
    
    if ([selectedLabel isEqualToString:CLOUDSYNC_ONOFF])
    {
        PFUser *user = [PFUser currentUser];
        IKU_log(@"switchChanged : user = |%@|", user); 
        if (user == nil)
        {
            __block UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Info"
                                                                                   message:@"You must be logged in to \nenable Cloud Sync"
                                                                            preferredStyle:UIAlertControllerStyleAlert];
        
            UIAlertAction *ok = [UIAlertAction actionWithTitle:@"OK"
                                                         style:UIAlertActionStyleDefault
                                                       handler:^(UIAlertAction *action)
                                                       {
                                                           [switchControl setOn:NO animated:YES];
                                                           [alert dismissViewControllerAnimated:YES completion:nil];
                                                       }];
        
            [alert addAction:ok];
            [self presentViewController:alert animated:YES completion:nil];
        }
        else
        {
            [self.userDefaults setBool:switchControl.on forKey:[self.labelKey objectForKey:selectedLabel]];
            [self.userDefaults synchronize];
        }
    }
}

- (void)updatePFUserToUserDefaults:(PFUser *)user
{
    [self.userDefaults setObject:@"" forKey:CLOUDSYNC_SESSIONTOKEN_PREFERENCE];
    if (user != nil)
    {
        [self.userDefaults setObject:[user sessionToken] forKey:CLOUDSYNC_SESSIONTOKEN_PREFERENCE];
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
    
    if ([text isEqualToString:SIGNUP_CLOUDSYNC])
    {
        cell.textLabel.text = text;
        cell.textLabel.textColor = [UIColor blueColor];
        [cell setAccessoryType:UITableViewCellAccessoryNone];
    }
    else if ([text isEqualToString:CLOUDSYNC_ONOFF])
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
    NSUInteger sectionNumber = [indexPath indexAtPosition:0];
    NSUInteger rowNumber = [indexPath indexAtPosition:1];
    if ((sectionNumber == SIGNUP_CLOUDSYNC_SECTION) && (rowNumber == SIGNUP_CLOUDSYNC_ROW))
    {
        IKULoginViewController *logInController = [[IKULoginViewController alloc] init];
        logInController.emailAsUsername = YES;
        logInController.delegate = self;
        
        IKUSignUpViewController *signUpController = [[IKUSignUpViewController alloc] init];
        signUpController.emailAsUsername = YES;
        logInController.signUpController = signUpController;
        
        [self presentViewController:logInController animated:YES completion:nil];
    }
}

#pragma mark - Login View Delegate

- (void)logInViewController:(PFLogInViewController *)logInController didLogInUser:(PFUser *)user
{
    [self dismissViewControllerAnimated:YES completion:nil];
    [self updatePFUserToUserDefaults:user];
}

- (void)logInViewControllerDidCancelLogIn:(PFLogInViewController *)logInController
{
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)logInViewController:(PFLogInViewController *)logInController didFailToLogInWithError:(NSError *)error
{
    IKU_log(@"didFailToLogInWithError : error = |%@|", error);
}

#pragma mark - Sign Up View Delegate

- (void)signUpViewController:(PFSignUpViewController *)signUpController didSignUpUser:(PFUser *)user
{
    [self dismissViewControllerAnimated:YES completion:nil];
    [self updatePFUserToUserDefaults:user];
}

- (void)signUpViewControllerDidCancelSignUp:(PFSignUpViewController *)signUpController
{
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)signUpViewController:(PFSignUpViewController *)signUpController didFailToSignUpWithError:(NSError *)error
{
    IKU_log(@"didFailToSignUpWithError : error = |%@|", error);
}

@end
