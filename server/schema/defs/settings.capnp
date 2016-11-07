#
#  settings.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xcd64c162aea242f4;

struct ServerSettings {
  dirPrefix           @0 : Text;
  fragmentCountLimit  @1 : Int32;
  serverChannel       @2 : Text;
  serverStreamId      @3 : Int32;
  engineChannel       @4 : Text;
  engineStreamId      @5 : Int32;
  traceChannel        @6 : Text;
  traceStreamId       @7 : Int32;
}

struct DiscoverydSettings {
  dbPath              @0 : Text;
  dbSize              @1 : Int32;
  dirPrefix           @2 : Text;
  fragmentCountLimit  @3 : Int32;
  serverChannel       @4 : Text;
  serverStreamId      @5 : Int32;
  engineChannel       @6 : Text;
  engineStreamId      @7 : Int32;
}

struct EventLoggerSettings {
  dbPath              @0 : Text;
  metaDBSize          @1 : Int32;
  dirPrefix           @2 : Text;
  fragmentCountLimit  @3 : Int32;
  serverChannel       @4 : Text;
  serverStreamId      @5 : Int32;
  loggerChannel       @6 : Text;
  loggerStreamId      @7 : Int32;
}

struct RequestHistogrammerSettings {
  dbPath              @0 : Text;
  metaDBSize          @1 : Int32;
  histDBSize          @2 : Int32;
  maxHistVal          @3 : Int32;
  lowerInterval       @4 : Int32;
  upperInterval       @5 : Int32;
}
