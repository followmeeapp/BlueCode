#
#  join.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xa6e63b6c9b7e6567;

using import "section.capnp".Section;

struct JoinRequest {
  telephone @0 :Text;
}

struct JoinResponse {
  status   @0 :Status;
  user     @1 :import "user.capnp".User;
  card     @2 :import "card.capnp".Card;
  sections @3 :List(Section);

  enum Status {
    new      @0;
    existing @1;
  }
}
