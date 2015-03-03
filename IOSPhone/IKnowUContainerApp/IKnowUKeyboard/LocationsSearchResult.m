//
//  LocationsSearchResult.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-04.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "LocationsSearchResult.h"

@implementation LocationsSearchResult

- (id)init
{
    self = [super init];
    if (self)
    {
        self.name = @"";
        self.address = @"";
        self.street = @"";
        self.city = @"";
        self.province = @"";
        self.country = @"";
        self.distance = @"";
        self.rating = @"";
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"obj = %@ name[%@] address[%@] street[%@] "
            "city[%@] province[%@] country[%@] distance[%@] rating[%@] ",
            [super description], self.name, self.address, self.street,
            self.city, self.province, self.country, self.distance, self.rating];
}

@end
