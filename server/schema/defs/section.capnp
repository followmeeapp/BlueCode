#
#  section.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xda4003684f556034;

using import "card_info.capnp".CardInfo;
using import "section_info.capnp".SectionInfo;

struct Section {
  id    @0 :Int64;
  info  @1 :SectionInfo;
  cards @2 :List(CardInfo);
}

struct CreateSectionRequest {
  card @0 :Section;
}
