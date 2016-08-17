#
#  discover_info.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xfa17781cc7ab2fdc;

struct DiscoverInfo {
  requestId                     @0 :Int64;
  type                          @1 :Type;
  shouldSuppressSectionCreation @2 :Bool = false;

  enum Type {
    unknown @0;
    discover @1;
    deactivateDevice @2;
  }
}
