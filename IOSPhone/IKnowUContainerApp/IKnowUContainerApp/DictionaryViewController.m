//
//  DictionaryViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-10-01.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "SharedDefaults.h"
#import "MessagesUtil.h"
#import "AddDictWordViewController.h"
#import "DictionaryViewController.h"

#define ADDDICTWORD @"AddDictWord"

#define REFRESH_DESCRIPTION @"Refresh local dictionary with words stored in cloud"

@interface DictionaryViewController ()

@property NSMutableArray *engineDictionaryWords;
@property (strong, nonatomic) NSUserDefaults *userDefaults;

@end

@implementation DictionaryViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");
    
    self.dictionaryTable.dataSource = self;
    self.dictionaryTable.delegate = self;
    
    self.userDefaults = [[NSUserDefaults alloc] initWithSuiteName:@"group.wordlogic.IKnowUContainerApp"];
    [self loadDictionary];
    
    self.navigationItem.rightBarButtonItem = self.editButtonItem;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    IKU_log(@"viewWillAppear : ");
    
    [self.dictionaryTable deselectRowAtIndexPath:[self.dictionaryTable indexPathForSelectedRow] animated:YES];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(becomeActive)
                                                 name:UIApplicationDidBecomeActiveNotification
                                               object:nil];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
    
    IKU_log(@"viewWillDisappear : ");
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:UIApplicationDidBecomeActiveNotification
                                                  object:nil];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)setEditing:(BOOL)editing animated:(BOOL)animated
{
    if (editing != self.editing)
    {
        [super setEditing:editing animated:animated];
        [self.dictionaryTable setEditing:editing animated:animated];
        
        NSArray *indexes = [NSArray arrayWithObject:[NSIndexPath indexPathForRow:self.engineDictionaryWords.count inSection:0]];
        if (editing == YES)
        {
            [self.dictionaryTable insertRowsAtIndexPaths:indexes withRowAnimation:UITableViewRowAnimationLeft];
        }
        else
        {
            [self.dictionaryTable deleteRowsAtIndexPaths:indexes withRowAnimation:UITableViewRowAnimationLeft];
        }
    }
}

#pragma mark - Table View DataSource

- (NSString *)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section
{
    if (section == 1)
    {
        return REFRESH_DESCRIPTION;
    }
    return nil;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
    if (section == 0)
    {
        return @"Manage your personal dictionary";
    }
    return nil;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 2;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    if (section == 0)
    {
        NSInteger count = self.engineDictionaryWords.count;
        if (self.editing)
        {
            count = count + 1;
        }
    
        return count;
    }
    else
    {
        return 1;
    }
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Create an identifier for this type of cell
    static NSString *CellIdentifier = @"Cell";
    
    // Get a cell of this type from the re-use queue or create one
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil)
    {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier];
    }
    
    NSUInteger sectionNumber = [indexPath indexAtPosition:0];
    if (sectionNumber == 0)
    {
        if (indexPath.row < self.engineDictionaryWords.count)
        {
            NSString *object = self.engineDictionaryWords[indexPath.row];
            cell.textLabel.text = [object description];
            cell.textLabel.textColor = [UIColor blackColor];
            cell.editingAccessoryType = UITableViewCellAccessoryNone;
            if (self.editing)
            {
                [cell setSelectionStyle:UITableViewCellSelectionStyleNone];
            }
        }
        else
        {
            cell.textLabel.text = @"Add new word...";
            cell.textLabel.textColor = [UIColor lightGrayColor];
            cell.editingAccessoryType = UITableViewCellAccessoryDisclosureIndicator;
        }
    }
    else if (sectionNumber == 1)
    {
        cell.textLabel.text = @"Refresh";
        cell.textLabel.textColor = [UIColor blueColor];
        [cell setAccessoryType:UITableViewCellAccessoryNone];
    }
    
    return cell;
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSUInteger sectionNumber = [indexPath indexAtPosition:0];
    if (sectionNumber == 0)
    {
        return YES;
    }
    else
    {
        // Return NO if you do not want the specified item to be editable.
        return NO;
    }
}

-(UITableViewCellEditingStyle)tableView:(UITableView *)tableView editingStyleForRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSUInteger sectionNumber = [indexPath indexAtPosition:0];
    if (sectionNumber == 0)
    {
        if (indexPath.row < self.engineDictionaryWords.count)
        {
            return UITableViewCellEditingStyleDelete;
        }
        else
        {
            return UITableViewCellEditingStyleInsert;
        }
    }
    else
    {
        return UITableViewCellEditingStyleNone;
    }
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSUInteger sectionNumber = [indexPath indexAtPosition:0];
    if (sectionNumber == 0)
    {
        if (editingStyle == UITableViewCellEditingStyleDelete)
        {
            // Remove word from local dictionary
            NSString *word = [self.engineDictionaryWords objectAtIndex:indexPath.row];
            [self.engineDictionaryWords removeObjectAtIndex:indexPath.row];
            [self.userDefaults setObject:self.engineDictionaryWords forKey:DICTIONARY_PREFERENCE];
            [self.userDefaults synchronize];
            
            BOOL cloudSyncEnabled = [self.userDefaults boolForKey:CLOUDSYNC_ONOFF_PREFERENCE];
            PFUser *user = [PFUser currentUser];
            if ((user != nil) && cloudSyncEnabled)
            {
                // Delete word from cloud storage
                PFQuery *query = [PFQuery queryWithClassName:@"Word"];
                [query whereKey:@"User" equalTo:user.objectId];
                [query whereKey:@"Word" equalTo:[word lowercaseString]];
                [query getFirstObjectInBackgroundWithBlock:^(PFObject *object, NSError *error)
                {
                    if (!object)
                    {
                        IKU_log(@"deleteWord : word not found");
                    }
                    else
                    {
                        [object deleteInBackgroundWithBlock:^(BOOL succeeded, NSError *error)
                        {
                            IKU_log(@"deleteWord : succeeded = |%d|", succeeded);
                            IKU_log(@"deleteWord : error     = |%@|", error);
                        }];
                    }
                }];
            }
        
            [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
        }
        else if (editingStyle == UITableViewCellEditingStyleInsert)
        {
            // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
        }
    }
}

#pragma mark - Table View Delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSUInteger sectionNumber = [indexPath indexAtPosition:0];
    if (sectionNumber == 0)
    {
        if ((indexPath.row == self.engineDictionaryWords.count) && self.editing)
        {
            [self performSegueWithIdentifier:ADDDICTWORD sender:self];
        }
    }
    else if (sectionNumber == 1)
    {
        if (indexPath.row == 0)
        {
            [self refreshFromCloud];
        }
    }
}

#pragma mark - Support methods

- (void)becomeActive
{
    IKU_log(@"becomeActive : ");
    
    [self loadDictionary];
    [self.dictionaryTable reloadData];
}

-(void)loadDictionary
{
    NSArray *dictionaryWords = [self.userDefaults stringArrayForKey:DICTIONARY_PREFERENCE];
    IKU_log(@"loadDictionary : dictionaryWords = |%@|", dictionaryWords);
    if (dictionaryWords != nil)
    {
        if (!self.engineDictionaryWords)
        {
            self.engineDictionaryWords = [[NSMutableArray alloc] init];
        }
        [self.engineDictionaryWords setArray:dictionaryWords];
    }
}

- (void)addWord:(NSString *)word
{
    if (!self.engineDictionaryWords)
    {
        self.engineDictionaryWords = [[NSMutableArray alloc] init];
    }
    [self.engineDictionaryWords insertObject:word atIndex:0];
    [self.dictionaryTable reloadData];
    [self.userDefaults setObject:self.engineDictionaryWords forKey:DICTIONARY_PREFERENCE];
    [self.userDefaults synchronize];
    
    // Add word to cloud storage
    BOOL cloudSyncEnabled = [self.userDefaults boolForKey:CLOUDSYNC_ONOFF_PREFERENCE];
    PFUser *user = [PFUser currentUser];
    if ((user != nil) && cloudSyncEnabled)
    {
        PFObject *pfObj = [PFObject objectWithClassName:@"Word"];
        pfObj[@"User"] = user.objectId;
        pfObj[@"Word"] = [word lowercaseString];
        [pfObj saveInBackgroundWithBlock:^(BOOL succeeded, NSError *error)
        {
            IKU_log(@"addWord : succeeded = |%d|", succeeded);
            IKU_log(@"addWord : error     = |%@|", error);
        }];
    }
}

- (void)refreshFromCloud
{
    BOOL cloudSyncEnabled = [self.userDefaults boolForKey:CLOUDSYNC_ONOFF_PREFERENCE];
    PFUser *user = [PFUser currentUser];
    if (user == nil)
    {        
        UIAlertController *alert = [MessagesUtil showTitleMessageOK:@"Info"
                                                            message:@"You must be logged in to \nrefresh from cloud"];
        [self presentViewController:alert animated:YES completion:nil];
    }
    else if (!cloudSyncEnabled)
    {
        UIAlertController *alert = [MessagesUtil showTitleMessageOK:@"Info"
                                                            message:@"You must enable Cloud Sync to \nrefresh from cloud"];
        [self presentViewController:alert animated:YES completion:nil];
    }
    else
    {
        if ([self.engineDictionaryWords count] > 0)
        {
            __block UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Warning"
                                                                                   message:@"This action will overwrite all words currently in local dictionary. Continue?"
                                                                            preferredStyle:UIAlertControllerStyleAlert];
            
            UIAlertAction *yes = [UIAlertAction actionWithTitle:@"Yes"
                                                         style:UIAlertActionStyleDefault
                                                       handler:^(UIAlertAction *action)
                                 {
                                     [self loadCloudWordsToDictionary:user];
                                     [alert dismissViewControllerAnimated:YES completion:nil];
                                 }];
            
            UIAlertAction *no = [UIAlertAction actionWithTitle:@"No"
                                                         style:UIAlertActionStyleDefault
                                                       handler:^(UIAlertAction *action)
                                 {
                                     [alert dismissViewControllerAnimated:YES completion:nil];
                                 }];
            
            [alert addAction:yes];
            [alert addAction:no];
            [self presentViewController:alert animated:YES completion:nil];
        }
        else
        {
            [self loadCloudWordsToDictionary:user];
        }
    }
}

- (void)loadCloudWordsToDictionary:(PFUser *)user
{
    [[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:YES];
    
    PFQuery *query = [PFQuery queryWithClassName:@"Word"];
    [query whereKey:@"User" equalTo:user.objectId];
    [query findObjectsInBackgroundWithBlock:^(NSArray *objects, NSError *error)
     {
         if (!error)
         {
             IKU_log(@"Successfully retrieved %lu words", (unsigned long)objects.count);
             [self.engineDictionaryWords removeAllObjects];
             for (PFObject *object in objects)
             {
                 NSString *word = [object objectForKey:@"Word"];
                 [self.engineDictionaryWords addObject:word];
             }
             
             [self.dictionaryTable reloadData];
             [self.userDefaults setObject:self.engineDictionaryWords forKey:DICTIONARY_PREFERENCE];
             [self.userDefaults synchronize];
         }
         else
         {
             IKU_log(@"Error: %@ %@", error, [error userInfo]);
         }
         
         [[UIApplication sharedApplication] setNetworkActivityIndicatorVisible:NO];
     }];
}

@end
