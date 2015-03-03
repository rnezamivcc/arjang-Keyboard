//
//  LocationsSearchResultBuilder.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-09-04.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "IKUMacros.h"
#import "LocationsSearchResult.h"
#import "LocationsSearchResultBuilder.h"

@implementation LocationsSearchResultBuilder

double distance(double lat1, double lon1, double lat2, double lon2, char unit)
{
    double theta = lon1 - lon2;
    double dist = sin(DEGREES_TO_RADIANS(lat1)) * sin(DEGREES_TO_RADIANS(lat2)) +
    cos(DEGREES_TO_RADIANS(lat1)) * cos(DEGREES_TO_RADIANS(lat2)) * cos(DEGREES_TO_RADIANS(theta));
    dist = acos(dist);
    dist = RADIANS_TO_DEGREES(dist);
    dist = dist * 60 * 1.1515;
    if (unit == 'K')
    {
        dist = dist * 1.609344;
    }
    else if (unit == 'N')
    {
        dist = dist * 0.8684;
    }
    return (dist);
}

+ (NSString *)getDistanceFromLocation:(CLLocation *)location lat:(double)lat lng:(double)lng
{
    double myLat = location.coordinate.latitude;
    double myLng = location.coordinate.longitude;
    
    NSLocale *locale = [NSLocale currentLocale];
    NSString *countryCode = [[locale objectForKey: NSLocaleCountryCode] lowercaseString];
    
    char unit = ([countryCode isEqualToString:@"us"]) ? 'M' : 'K';
    double dis = distance(lat, lng, myLat, myLng, unit);
    
    NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
    [formatter setPositiveFormat:@"#,##0.0"];
    
    return [NSString stringWithFormat:@"%@ %@", [formatter stringFromNumber:[NSNumber numberWithDouble:dis]],
            (([countryCode isEqualToString:@"us"]) ? @"mi" : @"k")];
}

+ (NSArray *)resultsFromJSON:(NSData *)objectNotation location:(CLLocation *)location error:(NSError **)error
{
    NSError *localError = nil;
    NSDictionary *parsedObject = [NSJSONSerialization JSONObjectWithData:objectNotation options:0 error:&localError];
    
    if (localError != nil)
    {
        *error = localError;
        return nil;
    }
    
    NSMutableArray *searchResults = [[NSMutableArray alloc] init];
    NSArray *results = [parsedObject valueForKey:@"results"];
    
    for (NSDictionary *resultDic in results)
    {
        LocationsSearchResult *sr = [[LocationsSearchResult alloc] init];
        
        for (NSString *key in resultDic)
        {
            if ([key isEqualToString:@"name"])
            {
                sr.name = [resultDic valueForKey:key];
            }
            else if ([key isEqualToString:@"formatted_address"])
            {
                sr.address = [resultDic valueForKey:key];
                
                NSArray *items = [sr.address componentsSeparatedByString:@","];
                NSUInteger count = [items count];
                
                if (count > 0)
                {
                    sr.street = [[items objectAtIndex:0] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
                }
                if (count > 1)
                {
                    sr.city = [[items objectAtIndex:1] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
                }
                if (count > 2)
                {
                    sr.province = [[items objectAtIndex:2] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
                }
                if (count > 3)
                {
                    sr.country = [[items objectAtIndex:3] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
                }
            }
            else if ([key isEqualToString:@"rating"])
            {
                id number = [resultDic valueForKey:key];
                sr.rating = (number != nil) ? [NSString stringWithFormat:@"Rating %@", number] : @"";
            }
            else if ([key isEqualToString:@"geometry"])
            {
                id locationData = [[resultDic valueForKey:key] valueForKey:@"location"];
                if ([locationData isKindOfClass:[NSDictionary class]])
                {
                    double lat = [[locationData valueForKey:@"lat"] doubleValue];
                    double lng = [[locationData valueForKey:@"lng"] doubleValue];
                    sr.distance = [self getDistanceFromLocation:location lat:lat lng:lng];
                }
            }
        }
        
        [searchResults addObject:sr];
    }
    
    return searchResults;
}

@end
