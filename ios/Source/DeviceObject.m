//
//  DeviceObject.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "DeviceObject.h"

#import "CardListBuilder.h"

@interface DeviceObject ()

@property (nonatomic, strong) NSArray *internalVisibleCards;
@property (nonatomic, strong) NSArray *internalHiddenCards;

@end

@implementation DeviceObject

+ (NSString *) primaryKey
{
    return @"id";
}

+ (NSDictionary *) defaultPropertyValues
{
    return @{
        @"lastSeen": [NSDate dateWithTimeIntervalSinceNow: 0],
        @"lastAnalytics": @0,
    };
}

+ (NSArray *) ignoredProperties
{
    return @[@"needsToBeCreated", @"internalVisibleCards", @"internalHiddenCards"];
}

- (NSArray *) visibleCards
{
    NSArray *cards = self.internalVisibleCards;

    if (!cards) [self loadCards];

    return self.internalVisibleCards;
}

- (NSArray *) hiddenCards
{
    NSArray *cards = self.internalHiddenCards;

    if (!cards) [self loadCards];

    return self.internalHiddenCards;
}

- (void)
updateVisibleCards: (NSArray *) visibleCards
hiddenCards:        (NSArray *) hiddenCards;
{
    self.cardData = [CardListBuilder dataWithVisibleCards: visibleCards hiddenCards: hiddenCards];

    // Clear internal cache.
    self.internalVisibleCards = nil;
    self.internalHiddenCards = nil;
}

- (void) loadCards
{
    NSData *data = self.cardData;

    self.internalVisibleCards = data ? [[[CardListBuilder visibleCardsFromData: data] reverseObjectEnumerator] allObjects] : @[];
    self.internalHiddenCards  = data ? [[[CardListBuilder hiddenCardsFromData: data] reverseObjectEnumerator] allObjects] : @[];
}

@end
