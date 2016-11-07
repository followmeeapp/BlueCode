#
#  hello.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xdf3ba06abc2888c3;

struct HelloRequest {
  uuid      @0 :Data; # should always be exactly 16 bytes
  version   @1 :Int32 = 0;
  publicKey @2 :Data; # should always be exactly 32 bytes
  nonce     @3 :Data; # should always be exactly 24 bytes
}

struct HelloResponse {
  deviceId           @0 :Int64;
  discoveryResponse  @1 :import "discovery.capnp".DiscoveryResponse;
  serverVersion      @2 :Int32;
  schemaVersion      @3 :Int32;
  databaseID         @4 :Int64; # this is a random number saved with the database on the server; only useful during development
  joinRequired       @5 :Bool = false;
  isClientCompatible @6 :Bool = true;
}
