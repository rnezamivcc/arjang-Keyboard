//
//  LocationsSearchResult.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-04.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface LocationsSearchResult : NSObject

@property (strong, nonatomic) NSString *name;
@property (strong, nonatomic) NSString *address;
@property (strong, nonatomic) NSString *street;
@property (strong, nonatomic) NSString *city;
@property (strong, nonatomic) NSString *province;
@property (strong, nonatomic) NSString *country;
@property (strong, nonatomic) NSString *distance;
@property (strong, nonatomic) NSString *rating;

@end
