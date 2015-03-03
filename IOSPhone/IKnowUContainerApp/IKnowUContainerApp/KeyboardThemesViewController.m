//
//  KeyboardThemesViewController.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-10-07.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "GraphicUtil.h"
#import "SharedDefaults.h"
#import "KeyboardThemesViewController.h"
#import "ThemeFactory.h"

#define BACKGROUND_0FFSET 10
#define BUTTON_0FFSET 18

@interface KeyboardThemesViewController ()

@property (strong, nonatomic) NSArray *themes;
@property (strong, nonatomic) NSMutableDictionary *themeImages;
@property (strong, nonatomic) UIView *sampleView;
@property (strong, nonatomic) NSString *selectedKey;
@property (strong, nonatomic) NSUserDefaults *userDefaults;

@end

@implementation KeyboardThemesViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    IKU_log(@"viewDidLoad : ");
    
    self.themes = [ThemeFactory allKeys];
    self.themeImages = [[NSMutableDictionary alloc] init];
    
    self.keyboardThemesTable.dataSource = self;
    self.keyboardThemesTable.delegate = self;
    
    self.userDefaults = [[NSUserDefaults alloc] initWithSuiteName:@"group.wordlogic.IKnowUContainerApp"];
    self.selectedKey = [self.userDefaults stringForKey:KEYBOARD_THEME_PREFERENCE];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

// Create a sample single key view based on theme
- (UIView *)sampleKeyboardView:(Theme *)theme height:(CGFloat)height
{
    if (!self.sampleView)
    {
        self.sampleView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, height - BACKGROUND_0FFSET, height - BACKGROUND_0FFSET)];
        
        UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
        CGFloat x = ((height - BACKGROUND_0FFSET) - (height - BUTTON_0FFSET)) / 2;
        button.frame = CGRectMake(x, x, height - BUTTON_0FFSET, height - BUTTON_0FFSET);
        button.backgroundColor = [UIColor darkGrayColor];
        [button setTitle:@"Q" forState:UIControlStateNormal];
        button.layer.cornerRadius = BUTTON_CORNER_RADIUS;
        button.clipsToBounds = YES;
        [self.sampleView addSubview:button];
    }
    [self.sampleView setBackgroundColor:UIColorFromRGB(theme.backgroundColor)];
    NSArray *subviews = self.sampleView.subviews;
    for (id view in subviews)
    {
        if ([view isKindOfClass:[UIButton class]])
        {
            UIButton *button = (UIButton *)view;
            [button setBackgroundColor:UIColorFromRGB(theme.keyColor)];
            [button setTitleColor:UIColorFromRGB(theme.keyTextColor) forState:UIControlStateNormal];
        }
    }
    
    return self.sampleView;
}

#pragma mark - Table View DataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Number of sections
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Number of rows
    return [self.themes count];
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
    NSUInteger rowNumber = [indexPath indexAtPosition:1];
    cell.textLabel.text = [self.themes objectAtIndex:rowNumber];
    
    UIImage *themeImage = [self.themeImages objectForKey:cell.textLabel.text];
    if (!themeImage)
    {
        // Create icon based on theme
        ThemeFactory *themeFactory = [ThemeFactory defaultThemeFactory];
        Theme *theme = [themeFactory getTheme:cell.textLabel.text];
        themeImage = [GraphicUtil imageWithView:[self sampleKeyboardView:theme height:cell.frame.size.height]];
        
        // Add rounded corners to the icon
        UIGraphicsBeginImageContextWithOptions(themeImage.size, NO, 1.0);
        CGRect bounds = CGRectMake(0, 0, themeImage.size.width, themeImage.size.width);
        [[UIBezierPath bezierPathWithRoundedRect:bounds cornerRadius:BUTTON_CORNER_RADIUS] addClip];
        [themeImage drawInRect:bounds];
        themeImage = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        
        // Cache icon
        [self.themeImages setValue:themeImage forKey:cell.textLabel.text];
    }
    cell.imageView.image = themeImage;
    
    if ((self.selectedKey == nil) && (rowNumber == 0))
    {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    }
    else if ([self.selectedKey isEqualToString:cell.textLabel.text])
    {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    }
    else
    {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }
    
    if (cell.accessoryType == UITableViewCellAccessoryCheckmark)
    {
        // Select the row with the checkmark
        [self.keyboardThemesTable selectRowAtIndexPath:indexPath animated:TRUE scrollPosition:UITableViewScrollPositionNone];
        [[self.keyboardThemesTable delegate] tableView:self.keyboardThemesTable didSelectRowAtIndexPath:indexPath];
    }
    
    return cell;
}

#pragma mark - Table View Delegate

-(NSIndexPath *)tableView:(UITableView *)tableView willSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSIndexPath *oldIndex = [self.keyboardThemesTable indexPathForSelectedRow];
    [self.keyboardThemesTable cellForRowAtIndexPath:oldIndex].accessoryType = UITableViewCellAccessoryNone;
    [self.keyboardThemesTable cellForRowAtIndexPath:indexPath].accessoryType = UITableViewCellAccessoryCheckmark;
    
    return indexPath;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSUInteger rowNumber = [indexPath indexAtPosition:1];
    self.selectedKey = [self.themes objectAtIndex:rowNumber];
    
    [self.userDefaults setObject:self.self.selectedKey forKey:KEYBOARD_THEME_PREFERENCE];
    [self.userDefaults synchronize];
}

@end
