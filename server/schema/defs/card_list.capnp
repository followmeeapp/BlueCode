#
#  card_list.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xb7596e04bfccd1ac;

struct CardList {
  visibleCards          @0 :List(Int64);
  visibleCardTimestamps @1 :List(Int64);
  hiddenCards           @2 :List(Int64);
  hiddenCardTimestamps  @3 :List(Int64);
}
