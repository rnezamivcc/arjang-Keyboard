//
//  ContactsUtil.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-07.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "Contact.h"
#import "ContactsUtil.h"
#import "DocumentUtil.h"
#import <AddressBook/AddressBook.h>

@implementation ContactsUtil

+ (NSArray *)loadContacts:(NSString *)searchBy
{
    NSMutableArray *contacts = [[NSMutableArray alloc] init];
    CFErrorRef error = NULL;
    ABAddressBookRef addressBook = ABAddressBookCreateWithOptions(NULL, &error);
    
    // request permission to search address book if we don't have already have it
    __block BOOL accessGranted = NO;
    if (ABAddressBookRequestAccessWithCompletion != NULL)
    {
        dispatch_semaphore_t sema = dispatch_semaphore_create(0);
        ABAddressBookRequestAccessWithCompletion(addressBook, ^(bool granted, CFErrorRef error)
        {
            accessGranted = granted;
            dispatch_semaphore_signal(sema);
        });
        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);        
    }
    else
    {
        accessGranted = YES;
    }
    
    // perform the search
    if (accessGranted)
    {
        CFArrayRef allPeople = ABAddressBookCopyArrayOfAllPeople(addressBook);
        CFIndex numberOfPeople = ABAddressBookGetPersonCount(addressBook);
        
        for(int i = 0; i < numberOfPeople; i++)
        {
            bool addContact = NO;
            Contact *contact = [[Contact alloc] init];
            ABRecordRef person = CFArrayGetValueAtIndex(allPeople, i);
            
            NSString *firstName = (__bridge_transfer NSString *)(ABRecordCopyValue(person, kABPersonFirstNameProperty));
            NSString *lastName = (__bridge_transfer NSString *)(ABRecordCopyValue(person, kABPersonLastNameProperty));
            [contact.firstName setString:(firstName != nil) ? firstName : @""];
            [contact.lastName setString:(lastName != nil) ? lastName : @""];
            
            if (!addContact)
            {
                addContact = ([DocumentUtil string:firstName containsSubstring:searchBy] ||
                              [DocumentUtil string:lastName containsSubstring:searchBy]);
            }
            
            NSData *imgData = (__bridge_transfer NSData*)ABPersonCopyImageDataWithFormat(person, kABPersonImageFormatThumbnail);
            contact.img = [UIImage imageWithData:imgData];
            
            ABMultiValueRef phoneNumbers = ABRecordCopyValue(person, kABPersonPhoneProperty);
            for (CFIndex i = 0; i < ABMultiValueGetCount(phoneNumbers); i++)
            {
                NSString *phoneNumber = (__bridge_transfer NSString *)ABMultiValueCopyValueAtIndex(phoneNumbers, i);
                CFStringRef locLabel = ABMultiValueCopyLabelAtIndex(phoneNumbers, i);
                NSString *label =(__bridge_transfer NSString*)ABAddressBookCopyLocalizedLabel(locLabel);
                
                if (!addContact)
                {
                    addContact = [DocumentUtil string:phoneNumber containsSubstring:searchBy];
                }

                [contact.phoneNumber addObject:[phoneNumber stringByTrimmingCharactersInSet:
                                                [NSCharacterSet whitespaceAndNewlineCharacterSet]]];
                [contact.phoneNumberLabel addObject:[label stringByTrimmingCharactersInSet:
                                                [NSCharacterSet whitespaceAndNewlineCharacterSet]]];
                if (locLabel != NULL) CFRelease(locLabel);
            }
            if (phoneNumbers != NULL) CFRelease(phoneNumbers);
            
            ABMutableMultiValueRef emails = ABRecordCopyValue(person, kABPersonEmailProperty);
            for (CFIndex i = 0; i < ABMultiValueGetCount(emails); i++)
            {
                NSString *email = (__bridge_transfer NSString *)ABMultiValueCopyValueAtIndex(emails, i);
                CFStringRef locLabel = ABMultiValueCopyLabelAtIndex(emails, i);
                NSString *label =(__bridge_transfer NSString*)ABAddressBookCopyLocalizedLabel(locLabel);
                
                if (!addContact)
                {
                    addContact = [DocumentUtil string:email containsSubstring:searchBy];
                }
                
                [contact.email addObject:[email stringByTrimmingCharactersInSet:
                                          [NSCharacterSet whitespaceAndNewlineCharacterSet]]];
                [contact.emailLabel addObject:[label stringByTrimmingCharactersInSet:
                                                     [NSCharacterSet whitespaceAndNewlineCharacterSet]]];
                if (locLabel != NULL) CFRelease(locLabel);
            }
            if (emails != NULL) CFRelease(emails);
            
            if (addContact)
            {
                [contacts addObject:contact];
            }
        }
        if (allPeople != NULL) CFRelease(allPeople);
    }
    if (addressBook != NULL) CFRelease(addressBook);
    
    return contacts;
}

@end
