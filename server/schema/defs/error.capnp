#
#  error.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xdb148e0707c7dfa7;

struct ErrorResponse {
  code    @0 :Code;
  message @1 :Text;

  enum Code {
    unknown @0;
    discoveryNotAvailable    @1;
    lastDiscoveryNotApplied  @2;
    previousBackupNotApplied @3;
  }
}
