//
//  SectionListBuilder.m
//  Follow
//
//  Created by Erich Ocean on 10/13/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "SectionListBuilder.h"

#include <iostream>
#include <string>

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/section_list.capnp.h"

@implementation SectionListBuilder

+ (NSData *) dataWithSections: (NSArray *) sections
{
    capnp::MallocMessageBuilder builder;
    SectionList::Builder sectionList = builder.initRoot<SectionList>();

    auto sectionsList = sectionList.initSections((unsigned int)sections.count);
    for (unsigned int idx=0, len=(unsigned int)sections.count; idx<len; ++idx) {
        sectionsList.set(idx, [sections[idx] integerValue]);
    }

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    return [NSData dataWithBytes: from length: size];
}

+ (NSArray *) sectionsFromData: (NSData *) data
{
    const void *bytes = [data bytes];
    size_t len = [data length];

    kj::ArrayPtr<const capnp::word> view((const capnp::word *)bytes, len / 8);
    auto reader = capnp::FlatArrayMessageReader(view);
    SectionList::Reader sectionList = reader.getRoot<SectionList>();
    auto sections = sectionList.getSections();

    NSMutableArray *ary = [NSMutableArray arrayWithCapacity: sections.size()];

    NSInteger idx = 0;
    for (uint64_t sectionId : sections) {
        ary[idx++] = @(sectionId);
    }

    return ary;
}

@end
