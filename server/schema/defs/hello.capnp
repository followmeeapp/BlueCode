#
#  hello.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xdf3ba06abc2888c3;

struct HelloRequest {
  uuid @0 :Data; # should always be exactly 16 bytes
}

struct HelloResponse {
  deviceId          @0 :Int64;
  discoveryResponse @1 :import "discovery.capnp".DiscoveryResponse;
}
