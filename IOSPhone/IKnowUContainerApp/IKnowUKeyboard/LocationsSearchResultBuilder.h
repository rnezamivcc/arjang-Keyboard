//
//  LocationsSearchResultBuilder.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-04.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>

@interface LocationsSearchResultBuilder : NSObject

+ (NSArray *)resultsFromJSON:(NSData *)objectNotation location:(CLLocation *)location error:(NSError **)error;

@end
