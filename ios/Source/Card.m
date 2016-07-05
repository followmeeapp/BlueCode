//
//  Card.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Card.h"

#import "CardDrop.h"

@implementation Card

+ (NSString *) primaryKey
{
    return @"id";
}

+ (NSArray *) indexedProperties {
    return @[@"guid", @"hasFacebook", @"hasTwitter", @"hasInstagram"];
}

+ (NSDictionary *) linkingObjectsProperties
{
    return @{
        @"cardDrops": [RLMPropertyDescriptor descriptorWithClass: CardDrop.class propertyName: @"card"],
    };
}

+ (NSDictionary *) defaultPropertyValues
{
    return @{
        @"guid": @0,
        @"version": @0, // Implies that the Card has never been saved to the server.
        @"hasFacebook" : @NO,
        @"hasTwitter": @NO,
        @"hasInstagram": @NO,
    };
}

@end
