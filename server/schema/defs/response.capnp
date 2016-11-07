#
#  response.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xaf2ccbe7831ea534;

struct Response {
  id @0:Int64;

  kind :union {
    unknown             @1 :Void;
    processing          @2 :Void;
    errorResponse       @3 :import "error.capnp".ErrorResponse;
    cardResponse        @4 :import "card.capnp".CardResponse;
    discoveryResponse   @5 :import "discovery.capnp".DiscoveryResponse;
    helloResponse       @6 :import "hello.capnp".HelloResponse;
    joinResponse        @7 :import "join.capnp".JoinResponse;
    backupListResponse  @8 :import "backup.capnp".BackupListResponse;
    backupResponse      @9 :import "backup.capnp".BackupResponse;
  }
}
