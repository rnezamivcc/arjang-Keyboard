//
//  ContactsMiniApp.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-08.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "IKUMacros.h"
#import "GraphicUtil.h"
#import "Contact.h"
#import "ContactsUtil.h"
#import "ContactsTableViewCell.h"
#import "ContactsMiniApp.h"

@implementation ContactsMiniApp

- (id)init
{
    self = [super init];
    if (self)
    {
        NSString *deviceExt = @"";
        CGFloat detailImageHeight = CONTACT_DETAIL_IMAGE_HEIGHT;
        CGFloat listImageHeight = CONTACT_LIST_IMAGE_HEIGHT;
        self.cellHeight = CONTACT_LIST_CELL_HEIGHT;
        if ([(NSString*)[UIDevice currentDevice].model hasPrefix:@"iPad"])
        {
            deviceExt = @"_iPad";
            detailImageHeight = CONTACT_DETAIL_IPAD_IMAGE_HEIGHT;
            listImageHeight = CONTACT_LIST_IPAD_IMAGE_HEIGHT;
            self.cellHeight = CONTACT_LIST_IPAD_CELL_HEIGHT;
        }
        IKU_log(@"init : deviceExt = |%@|", deviceExt);
        
        UIImage *image = [GraphicUtil imageWithImage:[UIImage imageNamed:@"contact_icon.png"]
                                        scaledToSize:CGSizeMake(MINI_APP_BAR_HEIGHT, MINI_APP_BAR_HEIGHT)];
        self.smallIcon = [[MiniAppBarIcon alloc] initWithImage:image];
        
        image = [GraphicUtil imageWithImage:[UIImage imageNamed:@"contact_icon2.png"]
                               scaledToSize:CGSizeMake(MINI_APP_MAIN_BAR_HEIGHT, MINI_APP_MAIN_BAR_HEIGHT)];
        self.bigIcon = [[MiniAppBarIcon alloc] initWithImage:image];
        
        image = [GraphicUtil imageWithImage:[UIImage imageNamed:@"contactdefault.png"]
                               scaledToSize:CGSizeMake(listImageHeight, listImageHeight)];
        self.defaultListImage = image;
        
        image = [GraphicUtil imageWithImage:[UIImage imageNamed:@"contactdefault.png"]
                               scaledToSize:CGSizeMake(detailImageHeight, detailImageHeight)];
        self.defaultDetailImage = image;
        
        self.contactsListView = [[[NSBundle mainBundle] loadNibNamed:[NSString stringWithFormat:@"%@%@", @"ContactsListView", deviceExt]
                                                               owner:self options:nil] objectAtIndex:0];
        self.contactsListView.contactTable.dataSource = self;
        self.contactsListView.contactTable.delegate = self;
        
        self.contactsDetailView = [[[NSBundle mainBundle] loadNibNamed:[NSString stringWithFormat:@"%@%@", @"ContactsDetailView", deviceExt]
                                                                 owner:self options:nil] objectAtIndex:0];
        self.contactsDetailView.delegate = self;
        
        self.contactsNotAvailableView = [[[NSBundle mainBundle] loadNibNamed:[NSString stringWithFormat:@"%@%@", @"ContactsNotAvailableView", deviceExt]
                                                                 owner:self options:nil] objectAtIndex:0];
    }
    return self;
}

- (void)engage:(UIView *)view
{
    self.contacts = [ContactsUtil loadContacts:self.search];
    IKU_log(@"engage : contacts = |%@|", self.contacts);
    
    self.parentView = view;
    
    [UIView transitionWithView:self.parentView
                      duration:0.4
                       options:UIViewAnimationOptionTransitionCrossDissolve
                    animations:^
                    {
                        [self clearParentView];
                        if (self.contacts == nil || [self.contacts count] == 0)
                        {
                            [self.parentView addSubview:self.contactsNotAvailableView];
                        }
                        else
                        {
                            [self.parentView addSubview:self.contactsListView];
                            [self.contactsListView.contactTable reloadData];
                        }
                    }
                    completion:nil];
}

- (void)resetContentDimensions:(CGRect)rect
{
    CGRect frame = self.contactsListView.frame;
    self.contactsListView.frame = CGRectMake(frame.origin.x, frame.origin.y, rect.size.width, rect.size.height);
    frame = self.contactsListView.contactTable.frame;    
    self.contactsListView.contactTable.frame = CGRectMake(frame.origin.x, frame.origin.y, rect.size.width, rect.size.height);
    
    frame = self.contactsDetailView.frame;
    self.contactsDetailView.frame = CGRectMake(frame.origin.x, frame.origin.y, rect.size.width, rect.size.height);
    
    frame = self.contactsNotAvailableView.frame;
    self.contactsNotAvailableView.frame = CGRectMake(frame.origin.x, frame.origin.y, rect.size.width, rect.size.height);
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
    return [self.contacts count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    // NSIndexPath contains an array of indexes.  For UITableView:
    //    indexAtPosition:0 is the section number
    //    indexAtPosition:1 is the row number
    
    // Create an identifier for this type of cell
    static NSString *CellIdentifier = @"contactCell";
    
    // Get a cell of this type from the re-use queue or create one
    ContactsTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil)
    {
        NSString *deviceExt = @"";
        if ([(NSString*)[UIDevice currentDevice].model hasPrefix:@"iPad"])
        {
            deviceExt = @"_iPad";
        }
        [tableView registerNib:[UINib nibWithNibName:[NSString stringWithFormat:@"%@%@", @"ContactsTableViewCell", deviceExt] bundle:nil] forCellReuseIdentifier:CellIdentifier];
        cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    }
    
    return cell;
}

- (void)tableView:(UITableView *)tableView willDisplayCell:(ContactsTableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Configure the cell...
    Contact *contact = [self.contacts objectAtIndex:[indexPath indexAtPosition:1]];
    UIImage *img = [contact img];
    if (img != nil)
    {
        cell.contactImageView.image = img;
    }
    else
    {
        cell.contactImageView.image = self.defaultListImage;
    }
    CGRect frame = cell.contactImageView.frame;
    CALayer *imageLayer = cell.contactImageView.layer;
    [imageLayer setCornerRadius:frame.size.width/2];
    [imageLayer setBorderWidth:0];
    [imageLayer setMasksToBounds:YES];
    
    NSString *name = [NSString stringWithFormat:@"%@ %@", contact.firstName, contact.lastName];
    [[cell nameLabel] setText:name];
    
    NSString *email = ([[contact email] count] > 0) ? [[contact email] objectAtIndex:0] : nil;
    NSString *phoneNumber = ([[contact phoneNumber] count] > 0) ? [[contact phoneNumber] objectAtIndex:0] : nil;
    [[cell emailLabel] setText:@""];
    [[cell phoneLabel] setText:@""];
    if (email != nil)
    {
        [[cell emailLabel] setText:email];
    }
    if (phoneNumber != nil)
    {
        if (email == nil)
        {
            [[cell emailLabel] setText:phoneNumber];
        }
        else
        {
            [[cell phoneLabel] setText:phoneNumber];
        }
    }
    
    [cell setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
}

#pragma mark - Table View Delegate

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Height of each table cell
    return self.cellHeight;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Get the selected cell
    UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
    [cell setAccessoryType:UITableViewCellAccessoryDisclosureIndicator];
    
    // Setup detail page
    Contact *contact = [self.contacts objectAtIndex:[indexPath indexAtPosition:1]];
    [self.contactsDetailView clear];
    [self.contactsDetailView setupDetails:contact defaultImage:self.defaultDetailImage];
    
    // Navigate to detail
    [UIView transitionWithView:self.parentView
                      duration:0.4
                       options:UIViewAnimationOptionTransitionCrossDissolve
                    animations:^
                    {
                        [self clearParentView];
                        [self.parentView addSubview:self.contactsDetailView];
                    }
                    completion:nil];
}

#pragma mark - Contacts Detail View Delegate

- (void)backAction
{
    [UIView transitionWithView:self.parentView
                      duration:0.4
                       options:UIViewAnimationOptionTransitionCrossDissolve
                    animations:^
                    {
                        [self clearParentView];
                        [self.parentView addSubview:self.contactsListView];
                        [self.contactsListView.contactTable reloadData];
                    }
                    completion:nil];
}

- (void)doneAction:(NSString *)s
{
    [self.delegate contactsTextSelectedAndDone:s];
}

- (void)clipAction:(NSString *)s
{
    [self.delegate contactsTextSelectedAndClip:s];
}

@end
