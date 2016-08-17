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
    for (uint64_t cardId : visibleCards) {
        ary[idx++] = @(cardId);
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
    for (uint64_t cardId : hiddenCards) {
        ary[idx++] = @(cardId);
    }

    return ary;
}

@end
