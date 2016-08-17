#
#  client_sync.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0x8169abf7260d5fbf;

using import "block.capnp".Block;
using import "remove.capnp".Removal;
using import "section.capnp".Section;

struct ClientSyncRequest {
  lastSection         @0 :Int64;
  lastBlockTimestamp  @1 :Int64;
  lastRemoveTimestamp @2 :Int64;
}

struct ClientSyncResponse {
  sections @0 :List(Section);
  blocks   @1 :List(Block);
  removals @2 :List(Removal);
}
