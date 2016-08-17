//
//  UserObject.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "UserObject.h"

@implementation UserObject

+ (NSString *) primaryKey
{
    return @"id";
}

+ (NSDictionary *) defaultPropertyValues
{
    return @{
        @"activeDeviceId": @0,
        @"cardId": @0,
    };
}

@end
