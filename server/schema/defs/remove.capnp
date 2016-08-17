#
#  remove.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xd54792e4a4fd1f7e;

struct Removal {
  timestamp @0 :Int64;
  cardId    @1 :Int64;
  sectionId @2 :Int64;
}

struct RemoveRequest {
  cardId    @0 :Int64;
  sectionId @1 :Int64;
}

struct RemoveResponse {
  removal @0 :Removal;
}
