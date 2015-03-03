//
//  MAData.m
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-13.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "MAData.h"

@implementation MAData

- (id)init
{
    self = [super init];
    if (self)
    {
        self.s = [[NSMutableString alloc] init];
    }
    return self;
}

- (void)reset
{
    [self.s setString:@""];
    self.miniAppCategory = kNoMiniAppCategory;
    self.miniAppAction = kNoMiniAppAction;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"obj = %@ s[%@] "
            "miniAppCategory[%lu] miniAppAction[%lu]",
            [super description], self.s, (unsigned long)self.miniAppCategory, (unsigned long)self.miniAppAction];
}

@end
