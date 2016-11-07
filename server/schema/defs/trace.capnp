#
#  trace.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xcad9c64cbca23db2;

struct Trace {
  struct Event {
    enum Function {
      unknown                     @0;
      handleClientMessage         @1;
      readRequest                 @2;
      prepareResponse             @3;
      finalizeResponse            @4;
      finalizeErrorResponse       @5;
      sendResponse                @6;
      prepareTrace                @7;

      handleInvalidMessage        @8;
      handleRequestNotImplemented @9;
      handleHelloRequest          @10;
      handleCardRequest           @11;
      handleJoinRequest           @12;
      handleCreateCardRequest     @13;
      handleUpdateCardRequest     @14;
      handleCreateBackupRequest   @15;
      handleBackupListRequest     @16;
      handleBackupRequest         @17;
    }

    enum Type {
      begin @0;
      end   @1;
      throw @2;
    }

    timestamp @0 :Int64;
    function  @1 :Function;
    type      @2 :Type;
  }

  requestId  @0 :Int64;
  deviceId   @1 :Int64;
  timestamp  @2 :Int64;
  duration   @3 :Int64;

  stackTrace  @4 :Text;
  requestData @5 :Data;
  error       @6 :Text;

  events @7 :List(Event);
}
