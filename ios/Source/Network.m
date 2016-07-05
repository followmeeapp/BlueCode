//
//  Network.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Network.h"

@implementation Network

+ (NSString *) primaryKey
{
    return @"id";
}

+ (NSArray *) indexedProperties
{
    return @[@"type"];
}

// TODO(Erich): Object properties cannot be made required, but '+[Network requiredProperties]' included 'card'
+ (NSArray *) requiredProperties
{
//    return @[@"username", @"card"];
    return @[@"username"];
}

@end
