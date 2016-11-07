#
#  user.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0x87d7ea5b44418829;

struct User {
  id @0 :Int64;

  telephone     @1 :Text;
  email         @2 :Text;
  emailVerified @3 :Bool;

  activeDevice     @4 :Int64 = 0;
  activeDeviceUUID @5 :Data;      # should be 16 bytes
  card             @6 :Int64 = 0;
}
