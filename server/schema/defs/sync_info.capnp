#
#  sync_info.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0x8fcf8fb2ae97aaa7;

struct SyncInfo {
  cardsVersion     @0 :Int64 = 0;
  lastCardTimestamp         @1 :Int64 = 0;
  lastDeviceUpdateTimestamp @2 :Int64 = 0;
}
