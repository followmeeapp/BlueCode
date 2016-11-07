//
//  CardListBuilder.m
//  Follow
//
//  Created by Erich Ocean on 7/30/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "CardListBuilder.h"

#include <iostream>
#include <string>

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/card_list.capnp.h"

@implementation CardListBuilder

+ (NSData *)
dataWithVisibleCards:  (NSArray *) visibleCards
visibleCardTimestamps: (NSArray *) visibleCardTimestamps
hiddenCards:           (NSArray *) hiddenCards
hiddenCardTimestamps:  (NSArray *) hiddenCardTimestamps
{
    capnp::MallocMessageBuilder builder;
    CardList::Builder cardList = builder.initRoot<CardList>();

    auto visibleCardsList = cardList.initVisibleCards((unsigned int)visibleCards.count);
    for (unsigned int idx=0, len=(unsigned int)visibleCards.count; idx<len; ++idx) {
        visibleCardsList.set(idx, [visibleCards[idx] integerValue]);
    }

    if (visibleCardTimestamps) {
        auto visibleCardTimestampsList = cardList.initVisibleCardTimestamps((unsigned int)visibleCardTimestamps.count);
        for (unsigned int idx=0, len=(unsigned int)visibleCardTimestamps.count; idx<len; ++idx) {
            visibleCardTimestampsList.set(idx, [visibleCardTimestamps[idx] integerValue]);
        }
    }

    auto hiddenCardsList = cardList.initHiddenCards((unsigned int)hiddenCards.count);
    for (unsigned int idx=0, len=(unsigned int)hiddenCards.count; idx<len; ++idx) {
        hiddenCardsList.set(idx, [hiddenCards[idx] integerValue]);
    }

    if (hiddenCardTimestamps) {
        auto hiddenCardTimestampsList = cardList.initHiddenCardTimestamps((unsigned int)hiddenCardTimestamps.count);
        for (unsigned int idx=0, len=(unsigned int)hiddenCardTimestamps.count; idx<len; ++idx) {
            hiddenCardTimestampsList.set(idx, [hiddenCardTimestamps[idx] integerValue]);
        }
    }

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    return [NSData dataWithBytes: from length: size];
}

+ (NSData *)
dataWithVisibleCards: (NSArray *) visibleCards
hiddenCards:          (NSArray *) hiddenCards
{
    return [self dataWithVisibleCards:  visibleCards
                 visibleCardTimestamps: nil
                 hiddenCards:           hiddenCards
                 hiddenCardTimestamps:  nil         ];
}

+ (NSArray *) visibleCardsFromData: (NSData *) data
{
    const void *bytes = [data bytes];
    size_t len = [data length];

    kj::ArrayPtr<const capnp::word> view((const capnp::word *)bytes, len / 8);
    auto reader = capnp::FlatArrayMessageReader(view);
    CardList::Reader cardList = reader.getRoot<CardList>();
    auto visibleCards = cardList.getVisibleCards();

    NSMutableArray *ary = [NSMutableArray arrayWithCapacity: visibleCards.size()];

    NSInteger idx = 0;
    for (int64_t cardId : visibleCards) {
        ary[idx++] = @(cardId);
    }

    return ary;
}

+ (NSArray *) visibleCardTimestampsFromData: (NSData *) data
{
    const void *bytes = [data bytes];
    size_t len = [data length];

    kj::ArrayPtr<const capnp::word> view((const capnp::word *)bytes, len / 8);
    auto reader = capnp::FlatArrayMessageReader(view);
    CardList::Reader cardList = reader.getRoot<CardList>();
    auto visibleCardTimestamps = cardList.getVisibleCardTimestamps();

    NSMutableArray *ary = [NSMutableArray arrayWithCapacity: visibleCardTimestamps.size()];

    NSInteger idx = 0;
    for (int64_t timestamp : visibleCardTimestamps) {
        ary[idx++] = @(timestamp);
    }

    return ary;
}

+ (NSArray *) hiddenCardsFromData: (NSData *) data
{
    const void *bytes = [data bytes];
    size_t len = [data length];

    kj::ArrayPtr<const capnp::word> view((const capnp::word *)bytes, len / 8);
    auto reader = capnp::FlatArrayMessageReader(view);
    CardList::Reader cardList = reader.getRoot<CardList>();

    auto hiddenCards = cardList.getHiddenCards();

    NSMutableArray *ary = [NSMutableArray arrayWithCapacity: hiddenCards.size()];

    NSInteger idx = 0;
    for (int64_t cardId : hiddenCards) {
        ary[idx++] = @(cardId);
    }

    return ary;
}

+ (NSArray *) hiddenCardTimestampsFromData: (NSData *) data
{
    const void *bytes = [data bytes];
    size_t len = [data length];

    kj::ArrayPtr<const capnp::word> view((const capnp::word *)bytes, len / 8);
    auto reader = capnp::FlatArrayMessageReader(view);
    CardList::Reader cardList = reader.getRoot<CardList>();
    auto hiddenCardTimestamps = cardList.getHiddenCardTimestamps();

    NSMutableArray *ary = [NSMutableArray arrayWithCapacity: hiddenCardTimestamps.size()];

    NSInteger idx = 0;
    for (int64_t timestamp : hiddenCardTimestamps) {
        ary[idx++] = @(timestamp);
    }

    return ary;
}

@end
