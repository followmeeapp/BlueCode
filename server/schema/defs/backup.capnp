#
#  backup.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0x9be9b694bb54e540;

struct Backup {
  timestamp @0 :Int64;
  data      @1 :Data;  # This is the backup data itself, and is opaque to the server.
}

struct CreateBackupRequest {
  previousBackup @0 :Int64;
  backup         @1 :Backup;
}

struct BackupRequest {
  timestamp @0 :Int64 = 0; # Zero implies "get most recent backup".
}

struct BackupResponse {
  backup @0 :Backup;
}

struct BackupListRequest {}
  
struct BackupListResponse {
  backups @0 :List(Backup);
}

struct BackupCard {
  id @0 :Int64;
  timestamp @1 :Int64;

  fullName @2 :Text;
  location @3 :Text;

  isBLECard      @4 :Bool = false;
  isBlueCardLink @5 :Bool = false;
}

struct ClientBackup {
  timestamp @0 :Int64;

  kind :union {
    unknown        @1 :Void;
    backupCardList @2 :List(BackupCard);
  }
}
