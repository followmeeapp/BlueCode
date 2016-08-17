//
//  DeviceObject.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "DeviceObject.h"

@implementation DeviceObject

+ (NSString *) primaryKey
{
    return @"id";
}

+ (NSDictionary *) defaultPropertyValues
{
    return @{
        @"lastSeen": [NSDate dateWithTimeIntervalSinceNow: 0],
    };
}

@end
