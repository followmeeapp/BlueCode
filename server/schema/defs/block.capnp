#
#  block.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0x80b301d2f5eb2404;

struct Block {
  timestamp @0 :Int64;
  cardId    @1 :Int64;
}

struct BlockRequest {
  cardId @0 :Int64;
}

struct BlockResponse {
  block @0 :Block;
}
