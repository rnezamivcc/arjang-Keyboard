//
//  LocationViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-26.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "SharedDefaults.h"
#import "IKUFormTableViewCell.h"
#import "LocationViewController.h"

@interface LocationViewController ()

@property (strong, nonatomic) UITextField *activeField;
@property (strong, nonatomic) NSUserDefaults *userDefaults;

@end

@implementation LocationViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");
    
    [self registerForKeyboardNotifications];
    
    self.locationTable.dataSource = self;
    self.locationTable.delegate = self;
    
    self.userDefaults = [[NSUserDefaults alloc] initWithSuiteName:@"group.wordlogic.IKnowUContainerApp"];
    
    self.apartmentNumber = [self getTextForKey:APARTMENT_NUMBER_PREFERENCE];
    self.address = [self getTextForKey:ADDRESS_PREFERENCE];
    self.city = [self getTextForKey:CITY_PREFERENCE];
    self.provinceState = [self getTextForKey:PROVINCE_STATE_PREFERENCE];
    self.country = [self getTextForKey:COUNTRY_PREFERENCE];
    self.postalZipCode = [self getTextForKey:POSTAL_ZIP_CODE_PREFERENCE];
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
    self.locationTable.contentInset = contentInsets;
    self.locationTable.scrollIndicatorInsets = contentInsets;
    
    CGRect aRect = self.view.frame;
    aRect.size.height -= kbSize.height;
    if (!CGRectContainsPoint(aRect, self.activeField.frame.origin))
    {
        [self.locationTable scrollRectToVisible:self.activeField.frame animated:YES];
    }
}

// Called when the UIKeyboardWillHideNotification is sent
- (void)keyboardWillBeHidden:(NSNotification *)aNotification
{
    UIEdgeInsets contentInsets = UIEdgeInsetsZero;
    self.locationTable.contentInset = contentInsets;
    self.locationTable.scrollIndicatorInsets = contentInsets;
}

#pragma mark - Table View DataSource

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
    return @"Setup your home address here for quick access to your information";
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Return the number of sections.
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Return the number of rows in the section.
    return 6;
}

// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Create an identifier for this type of cell
    static NSString *CellIdentifier = @"Cell";
    
    // Get a cell of this type from the re-use queue or create one
    IKUFormTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    
    if (cell == nil)
    {
        cell = [[IKUFormTableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier];
        switch (indexPath.row)
        {
            case 0: { cell.textField = [self makeTextField:self.apartmentNumber placeholder:@"Apartment #"]; break; }
            case 1: { cell.textField = [self makeTextField:self.address placeholder:@"Address"]; break; }
            case 2: { cell.textField = [self makeTextField:self.city placeholder:@"City"]; break; }
            case 3: { cell.textField = [self makeTextField:self.provinceState placeholder:@"Province/State"]; break; }
            case 4: { cell.textField = [self makeTextField:self.country placeholder:@"Country"]; break; }
            case 5: { cell.textField = [self makeTextField:self.postalZipCode placeholder:@"Postal/Zip Code"]; break; }
        }
        [cell.contentView addSubview:cell.textField];
    }
    else
    {
        switch (indexPath.row)
        {
            case 0: { cell.textField.placeholder = @"Apartment #"; cell.textField.text = self.apartmentNumber; break; }
            case 1: { cell.textField.placeholder = @"Address"; cell.textField.text = self.address; break; }
            case 2: { cell.textField.placeholder = @"City"; cell.textField.text = self.city; break; }
            case 3: { cell.textField.placeholder = @"Province/State"; cell.textField.text = self.provinceState; break; }
            case 4: { cell.textField.placeholder = @"Country"; cell.textField.text = self.country; break; }
            case 5: { cell.textField.placeholder = @"Postal/Zip Code"; cell.textField.text = self.postalZipCode; break; }
        }
    }
    
    cell.textField.tag = indexPath.row + 1;
    cell.selectionStyle = UITableViewCellSelectionStyleNone;
    
    NSDictionary *viewsDictionary = @{@"tf":cell.textField};
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
        
    // Workaround to dismiss keyboard when Done/Return is tapped
    [cell.textField addTarget:self action:@selector(textFieldFinished:) forControlEvents:UIControlEventEditingDidEndOnExit];
        
    // We want to handle textFieldDidEndEditing
    cell.textField.delegate = self;
    
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

- (NSString *)getTextForKey:(NSString *)key
{
    NSString *s = [self.userDefaults stringForKey:key];
    return (s != nil) ? s : @"";
}

- (void)setText:(NSString *)text forKey:(NSString *)key
{
    [self.userDefaults setObject:text forKey:key];
    [self.userDefaults synchronize];
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
    
    switch (textField.tag)
    {
        case 1: { self.apartmentNumber = textField.text; [self setText:self.apartmentNumber forKey:APARTMENT_NUMBER_PREFERENCE]; break; }
        case 2: { self.address = textField.text; [self setText:self.address forKey:ADDRESS_PREFERENCE]; break; }
        case 3: { self.city = textField.text; [self setText:self.city forKey:CITY_PREFERENCE]; break; }
        case 4: { self.provinceState = textField.text; [self setText:self.provinceState forKey:PROVINCE_STATE_PREFERENCE]; break; }
        case 5: { self.country = textField.text; [self setText:self.country forKey:COUNTRY_PREFERENCE]; break; }
        case 6: { self.postalZipCode = textField.text; [self setText:self.postalZipCode forKey:POSTAL_ZIP_CODE_PREFERENCE]; break; }
    }
}

@end
