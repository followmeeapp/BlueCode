#
#  card.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xd2e86bdff42f7c3a;

struct Network {
  type     @0 :Type;
  username @1 :Text;

  # IMPORTANT: MUST match the order/values in the Realm.io model!
  # typedef NS_ENUM(NSInteger, NetworkType) {
  #     FacebookType = 0,
  #     TwitterType,
  #     InstagramType,
  #     PokemonGoType,
  #     SnapchatType,
  #     GooglePlusType,
  #     YouTubeType,
  #     PinterestType,
  #     TumblrType,
  #     LinkedInType,
  #     PeriscopeType,
  #     VineType,
  #     SoundCloudType,
  #     SinaWeiboType,
  #     VKontakteType,
  # };
  enum Type {
    facebook    @0;
    twitter     @1;
    instagram   @2;
    pokemonGo   @3;
    snapchat    @4;
    googlePlus  @5;
    youTube     @6;
    pinterest   @7;
    tumblr      @8;
    linkedIn    @9;
    periscope   @10;
    vine        @11;
    soundClound @12;
    sinaWeibo   @13;
    vKontakte   @14;
  }
}

struct Card {
  id      @0 :Int64;
  version @1 :UInt32;

  fullName @2 :Text;
  location @3 :Text;
  bio      @4 :Text;

  avatarURLString     @5 :Text;
  backgroundURLString @6 :Text;

  hasFacebook   @7  :Bool;
  hasTwitter    @8  :Bool;
  hasInstagram  @9  :Bool;
  hasPokemonGo  @10 :Bool;
  hasSnapchat   @11 :Bool;
  hasGooglePlus @12 :Bool;
  hasYouTube    @13 :Bool;
  hasPinterest  @14 :Bool;
  hasTumblr     @15 :Bool;
  hasLinkedIn   @16 :Bool;
  hasPeriscope  @17 :Bool;
  hasVine       @18 :Bool;
  hasSoundCloud @19 :Bool;
  hasSinaWeibo  @20 :Bool;
  hasVKontakte  @21 :Bool;

  networks @22 :List(Network);
}

struct CreateCardRequest {
  card @0 :Card;
}

struct UpdateCardRequest {
  card @0 :Card;
}

struct CardRequest {
  id      @0 :Int64;
  version @1 :UInt32;
}

struct CardResponse {
  status @0 :Status;
  card   @1 :Card;

  enum Status {
    current @0;
    updated @1;
    created @2;
  }
}
