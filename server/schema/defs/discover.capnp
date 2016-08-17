#
#  discover.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xc00a04b463992979;

using import "card_info.capnp".CardInfo;
using import "discover_info.capnp".DiscoverInfo;
using import "sync_info.capnp".SyncInfo;

struct DiscoverRequest {
  timestamp    @0 :Int64 = 0;
  discoverInfo @1 :DiscoverInfo;
  syncInfo     @2 :import "sync_info.capnp".SyncInfo;
}

struct DiscoverResponse {
  timestamp    @0 :Int64;
  discoverInfo @1 :DiscoverInfo;
  syncInfo     @2 :SyncInfo;
  section      @3 :import "section_info.capnp".SectionInfo;
  cards        @4 :List(CardInfo);
}

struct DiscoverAckResponse {
  timestamp @0 :Int64;
}
