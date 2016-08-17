#
#  request.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xf8a7792a54809d33;

struct Request {
  id @0:Int64;

  kind :union {
    unknown             @1  :Void;
    helloRequest        @2  :import "hello.capnp".HelloRequest;
    cardRequest         @3  :import "card.capnp".CardRequest;
    discoveryRequest    @4  :import "discovery.capnp".DiscoveryRequest;
    joinRequest         @5  :import "join.capnp".JoinRequest;
    setDeviceRequest    @6  :import "set_device.capnp".SetDeviceRequest;
    createCardRequest   @7  :import "card.capnp".CreateCardRequest;
    updateCardRequest   @8  :import "card.capnp".UpdateCardRequest;
    blockRequest        @9  :import "block.capnp".BlockRequest;
    removeRequest       @10 :import "remove.capnp".RemoveRequest;
    clientSyncRequest   @11 :import "client_sync.capnp".ClientSyncRequest;
  }
}
