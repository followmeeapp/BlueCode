//
//  CardDrop.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "CardDrop.h"

@implementation CardDrop

+ (NSString *) primaryKey
{
    return @"id";
}

+ (NSArray *) indexedProperties
{
    return @[@"guid", @"cardGuid", @"displayName", @"hasFacebook", @"hasTwitter", @"hasInstagram"];
}

+ (NSDictionary *) defaultPropertyValues
{
    return @{
        @"guid": @0,
        @"version": @0, // Implies that the CardDrop has never been saved to the server.
        @"hasFacebook" : @NO,
        @"hasTwitter": @NO,
        @"hasInstagram": @NO,
    };
}

@end
