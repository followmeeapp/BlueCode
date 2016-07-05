//
//  CardFollow.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "CardFollow.h"

@implementation CardFollow

+ (NSString *) primaryKey
{
    return @"id";
}

+ (NSArray *) indexedProperties
{
    return @[@"cardGuid", @"displayName", @"hasFacebook", @"hasTwitter", @"hasInstagram"];
}

// TODO(Erich): Object properties cannot be made required, but '+[CardFollow requiredProperties]' included 'user'.
//+ (NSArray *) requiredProperties
//{
//    return @[@"user"];
//}

+ (NSDictionary *) defaultPropertyValues
{
    return @{
        @"version": @0, // Implies that the CardFollow has never been saved to the server.
        @"hasFacebook" : @NO,
        @"hasTwitter": @NO,
        @"hasInstagram": @NO,
    };
}

@end
