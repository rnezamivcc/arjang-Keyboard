//
//  AddDictWordViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-10-01.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "AddDictWordViewCell.h"
#import "AddDictWordViewController.h"
#import "DictionaryViewController.h"

@interface AddDictWordViewController ()

@property (strong, nonatomic) UITextField *activeField;

@end

@implementation AddDictWordViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");
    
    [self registerForKeyboardNotifications];
    
    self.tableView.dataSource = self;
    self.tableView.delegate = self;
    
    self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemSave
                                                  target:self action:@selector(saveWord:)];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Keyboard Notifications

- (void)registerForKeyboardNotifications

{
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(keyboardWasShown:)
                                                 name:UIKeyboardDidShowNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(keyboardWillBeHidden:)
                                                 name:UIKeyboardWillHideNotification
                                               object:nil];
}

// Called when the UIKeyboardDidShowNotification is sent.
- (void)keyboardWasShown:(NSNotification *)aNotification
{
    NSDictionary *info = [aNotification userInfo];
    CGSize kbSize = [[info objectForKey:UIKeyboardFrameBeginUserInfoKey] CGRectValue].size;
    
    if (kbSize.height == 0.0)
    {
        // Custom keyboard doesn't appear to have height when frame begins, so maybe at end?
        kbSize = [[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue].size;
    }
    
    UIEdgeInsets contentInsets = UIEdgeInsetsMake(0.0, 0.0, kbSize.height, 0.0);
    self.tableView.contentInset = contentInsets;
    self.tableView.scrollIndicatorInsets = contentInsets;
    
    CGRect aRect = self.view.frame;
    aRect.size.height -= kbSize.height;
    if (!CGRectContainsPoint(aRect, self.activeField.frame.origin))
    {
        [self.tableView scrollRectToVisible:self.activeField.frame animated:YES];
    }
}

// Called when the UIKeyboardWillHideNotification is sent
- (void)keyboardWillBeHidden:(NSNotification *)aNotification
{
    UIEdgeInsets contentInsets = UIEdgeInsetsZero;
    self.tableView.contentInset = contentInsets;
    self.tableView.scrollIndicatorInsets = contentInsets;
}

- (void)saveWord:(id)sender
{
    UINavigationController *navController = self.navigationController;
    DictionaryViewController *dictController = [navController.viewControllers objectAtIndex:0];
    
    NSIndexPath *indexPath = [NSIndexPath indexPathForRow:0 inSection:0];
    AddDictWordViewCell *cell = (AddDictWordViewCell *)[self.tableView cellForRowAtIndexPath:indexPath];
    
    NSString *word = cell.addWordTextField.text;
    if ([word length] > 0)
    {
        [dictController addWord:word];
        [navController popToViewController:dictController animated:YES];
    }
}

#pragma mark - Table View DataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Return the number of sections.
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Return the number of rows in the section.
    return 1;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    
    // Get a cell of this type from the re-use queue or create one
    UITextField *tf = nil;
    AddDictWordViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil)
    {
        cell = [[AddDictWordViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier];
        
        cell.selectionStyle = UITableViewCellSelectionStyleNone;
        
        tf = cell.addWordTextField = [self makeTextField:@"" placeholder:@"Add new word"];
        tf.tag = 1;
        [cell.contentView addSubview:cell.addWordTextField];
        
        NSDictionary *viewsDictionary = @{@"tf":tf};
        NSArray *constraint_V = [NSLayoutConstraint constraintsWithVisualFormat:@"V:|-[tf]-|"
                                                                            options:0
                                                                            metrics:nil
                                                                              views:viewsDictionary];
        NSArray *constraint_H = [NSLayoutConstraint constraintsWithVisualFormat:@"H:|-12-[tf]-12-|"
                                                                            options:0
                                                                            metrics:nil
                                                                              views:viewsDictionary];
        [cell.contentView addConstraints:constraint_V];
        [cell.contentView addConstraints:constraint_H];
        
        [tf addTarget:self action:@selector(textFieldFinished:) forControlEvents:UIControlEventEditingDidEndOnExit];
        
        tf.delegate = self;
    }
    else
    {
        tf = (UITextField *)[cell.contentView viewWithTag:1];
        tf.placeholder = @"Add new word";
        tf.text = @"";
    }
    
    return cell;    
}

#pragma mark - Table View Delegate

-(CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 44.0;
}

- (UITextField *)makeTextField:(NSString *)text placeholder:(NSString *)placeholder
{
    UITextField *tf = [[UITextField alloc] init];
    
    tf.translatesAutoresizingMaskIntoConstraints = NO;
    tf.clearButtonMode = UITextFieldViewModeWhileEditing;
    tf.placeholder = placeholder;
    tf.text = text;
    tf.autocorrectionType = UITextAutocorrectionTypeNo;
    tf.autocapitalizationType = UITextAutocapitalizationTypeNone;
    tf.adjustsFontSizeToFitWidth = YES;
    tf.textColor = [UIColor colorWithRed:56.0f / 255.0f green:84.0f / 255.0f blue:135.0f / 255.0f alpha:1.0f];
    
    return tf;
}

// Workaround to hide keyboard when Done is tapped
- (IBAction)textFieldFinished:(id)sender
{
    // [sender resignFirstResponder];
}

#pragma mark - Text Field Delegate

- (void)textFieldDidBeginEditing:(UITextField *)textField
{
    self.activeField = textField;    
}

// Textfield value changed, store the new value.
- (void)textFieldDidEndEditing:(UITextField *)textField
{
    self.activeField = nil;    
}

@end
