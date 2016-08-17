#
#  section_info.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xb802249b6859cf0e;

struct SectionInfo {
  kind :union {
    unknown          @0 :Void;
    timestampSection @1 :TimestampSection;
  }
}

struct TimestampSection {
  timestamp @0 :Int64;
  lastCard  @1 :Int64;
}
