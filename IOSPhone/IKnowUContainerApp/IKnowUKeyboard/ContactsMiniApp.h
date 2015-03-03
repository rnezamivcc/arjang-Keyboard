//
//  ContactsMiniApp.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-08.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "MiniAppBarIcon.h"
#import "MiniAppBase.h"
#import "ContactsListView.h"
#import "ContactsDetailView.h"

@protocol ContactsMiniAppDelegate

- (void)contactsTextSelectedAndDone:(NSString *)s;
- (void)contactsTextSelectedAndClip:(NSString *)s;

@end

@interface ContactsMiniApp : MiniAppBase <UITableViewDataSource, UITableViewDelegate, ContactsDetailViewDelegate>

@property (weak) id <ContactsMiniAppDelegate> delegate;
@property (nonatomic, strong) ContactsListView *contactsListView;
@property (nonatomic, strong) ContactsDetailView *contactsDetailView;
@property (nonatomic, strong) UIView *contactsNotAvailableView;
@property (nonatomic, strong) NSArray *contacts;
@property (nonatomic, strong) UIImage *defaultListImage;
@property (nonatomic, strong) UIImage *defaultDetailImage;
@property (nonatomic) CGFloat cellHeight;

@end
