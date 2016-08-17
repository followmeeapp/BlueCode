#
#  rpc.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0x9cb224a79c99320d;

struct RPC {
  device @0 :Int64;

  kind :union {
    unknown             @1 :Void;
    discoverRequest     @2 :import "discover.capnp".DiscoverRequest;
    discoverResponse    @3 :import "discover.capnp".DiscoverResponse;
    discoverAckResponse @4 :import "discover.capnp".DiscoverAckResponse;
    syncRequest         @5 :import "sync.capnp".SyncRequest;
    syncResponse        @6 :import "sync.capnp".SyncResponse;
    errorResponse       @7 :import "error.capnp".ErrorResponse;
  }
}
