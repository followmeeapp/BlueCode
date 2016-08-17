#
#  user.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0x87d7ea5b44418829;

struct User {
  id @0 :Int64;

  activeDevice     @1 :Int64 = 0;
  activeDeviceUUID @2 :Data;      # should be 16 bytes
  card             @3 :Int64 = 0;
}
