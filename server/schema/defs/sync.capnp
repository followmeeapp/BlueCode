#
#  sync.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xe4e4ebcfac670506;

using import "card_info.capnp".CardInfo;
using import "device_info.capnp".DeviceInfo;
using import "discover_info.capnp".DiscoverInfo;
using import "sync_info.capnp".SyncInfo;

struct SyncRequest {
  discoverInfo @0 :DiscoverInfo;

  wantsCardsAdvertisedByDevice @1 :Bool = false;
  latestRecentCard             @2 :Int64 = 0;
  latestDiscoveredDevice       @3 :Int64 = 0;
}

struct SyncResponse {
  discoverInfo @0 :DiscoverInfo;
  syncInfo     @1 :SyncInfo;

  cardsAdvertisedByDevice @2 :List(Int64);
  discoveredDevices       @3 :List(DeviceInfo);
  recentCards             @4 :List(CardInfo);
}
