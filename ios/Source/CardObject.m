//
//  CardObject.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "CardObject.h"

@implementation CardObject

+ (NSString *) primaryKey
{
    return @"id";
}

+ (NSArray *) indexedProperties {
    return @[@"version", @"isOwnCard"];
}

+ (NSDictionary *) defaultPropertyValues
{
    return @{
        @"version": @0, // Implies that the Card has never been saved to the server.
        @"isOwnCard": @NO,
        @"visibleSection": @0,
        @"hasFacebook": @NO,
        @"hasTwitter": @NO,
        @"hasInstagram": @NO,
        @"timestamp": [NSDate dateWithTimeIntervalSinceNow: 0],
    };
}

@end
