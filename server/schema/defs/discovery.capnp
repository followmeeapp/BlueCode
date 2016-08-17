#
#  discovery.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0x8bce6c366afb87bd;

using import "card_info.capnp".CardInfo;
using import "device_info.capnp".DeviceInfo;
using import "section.capnp".Section;

struct DiscoveryRequest {
  lastDiscovery @0 :Int64 = 0;

  cards         @1 :List(CardInfo);
  devices       @2 :List(DeviceInfo);
}

struct DiscoveryResponse {
  timestamp @0 :Int64;
  section   @1 :Section;
  cards     @2 :List(CardInfo);
}
