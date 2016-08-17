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
@property (nonatomic, strong) NSArray *internalHiddenCards;

@end

@implementation SectionObject

+ (NSString *) primaryKey
{
    return @"id";
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

- (void) loadCards
{
    NSData *data = self.cards;

    self.internalVisibleCards = [CardListBuilder visibleCardsFromData: data];
    self.internalHiddenCards  = [CardListBuilder hiddenCardsFromData: data];
}

+ (NSArray *) ignoredProperties
{
    return @[@"internalVisibleCards", @"internalHiddenCards"];
}

@end
