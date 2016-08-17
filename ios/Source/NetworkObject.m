//
//  NetworkObject.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "NetworkObject.h"

@implementation NetworkObject

+ (NSString *) primaryKey
{
    return @"guid";
}

+ (NSArray *) indexedProperties
{
    return @[@"type", @"cardId"];
}

// TODO(Erich): Object properties cannot be made required, but '+[Network requiredProperties]'
// would include 'card' if it could.
+ (NSArray *) requiredProperties
{
    return @[@"username"];
}

@end
