//
//  SectionObject.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "SectionObject.h"

#import "CardListBuilder.h"

@interface SectionObject ()

@property (nonatomic, strong) NSArray *internalVisibleCards;
@property (nonatomic, strong) NSArray *internalVisibleCardTimestamps;
@property (nonatomic, strong) NSArray *internalHiddenCards;
@property (nonatomic, strong) NSArray *internalHiddenCardTimestamps;

@end

@implementation SectionObject

+ (NSString *) primaryKey
{
    return @"id";
}

+ (NSArray *) ignoredProperties
{
    return @[@"internalVisibleCards", @"internalVisibleCardTimestamps", @"internalHiddenCards", @"internalHiddenCardTimestamps"];
}

- (NSArray *) visibleCards
{
    NSArray *cards = self.internalVisibleCards;

    if (!cards) [self loadCards];

    return self.internalVisibleCards;
}

- (NSArray *) visibleCardTimestamps
{
    NSArray *cards = self.internalVisibleCardTimestamps;

    if (!cards) [self loadCards];

    return self.internalVisibleCardTimestamps;
}

- (NSArray *) hiddenCards
{
    NSArray *cards = self.internalHiddenCards;

    if (!cards) [self loadCards];

    return self.internalHiddenCards;
}

- (NSArray *) hiddenCardTimestamps
{
    NSArray *cards = self.internalHiddenCardTimestamps;

    if (!cards) [self loadCards];

    return self.internalHiddenCardTimestamps;
}

- (void)
updateVisibleCards:    (NSArray *) visibleCards
visibleCardTimestamps: (NSArray *) visibleCardTimestamps
hiddenCards:           (NSArray *) hiddenCards
hiddenCardTimestamps:  (NSArray *) hiddenCardTimestamps
{
    self.cardData = [CardListBuilder dataWithVisibleCards:  visibleCards
                                     visibleCardTimestamps: visibleCardTimestamps
                                     hiddenCards:           hiddenCards
                                     hiddenCardTimestamps:  hiddenCardTimestamps];

    // Update internal cache.
    self.internalVisibleCards          = [visibleCards copy];
    self.internalVisibleCardTimestamps = [visibleCardTimestamps copy];
    self.internalHiddenCards           = [hiddenCards copy];
    self.internalHiddenCardTimestamps  = [hiddenCardTimestamps copy];
}

- (void) loadCards
{
    NSData *data = self.cardData;

    self.internalVisibleCards          = data ? [CardListBuilder visibleCardsFromData: data] : @[];
    self.internalVisibleCardTimestamps = data ? [CardListBuilder visibleCardTimestampsFromData: data] : @[];
    self.internalHiddenCards           = data ? [CardListBuilder hiddenCardsFromData: data] : @[];
    self.internalHiddenCardTimestamps  = data ? [CardListBuilder hiddenCardTimestampsFromData: data] : @[];
}

@end
