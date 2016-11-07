//
//  AnalyticsObject.m
//  Follow
//
//  Created by Erich Ocean on 10/6/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "AnalyticsObject.h"

@implementation AnalyticsObject

+ (NSArray *) indexedProperties
{
    return @[@"timestamp"];
}

+ (NSDictionary *) defaultPropertyValues
{
    return @{
        @"timestamp": @0,
    };
}

@end
