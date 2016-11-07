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
    snapchat    @3;
    googlePlus  @4;
    youTube     @5;
    pinterest   @6;
    tumblr      @7;
    linkedIn    @8;
    periscope   @9;
    vine        @10;
    soundClound @11;
    sinaWeibo   @12;
    vKontakte   @13;
  }
}

struct Card {
  id      @0 :Int64;
  version @1 :Int32;

  fullName @2 :Text;
  location @3 :Text;
  bio      @4 :Text;

  avatarURLString     @5 :Text;
  backgroundURLString @6 :Text;
  backgroundOpacity   @7 :Float32;
  rowOffset           @8 :Int32;

  hasFacebook   @9  :Bool;
  hasTwitter    @10 :Bool;
  hasInstagram  @11 :Bool;
  hasSnapchat   @12 :Bool;
  hasGooglePlus @13 :Bool;
  hasYouTube    @14 :Bool;
  hasPinterest  @15 :Bool;
  hasTumblr     @16 :Bool;
  hasLinkedIn   @17 :Bool;
  hasPeriscope  @18 :Bool;
  hasVine       @19 :Bool;
  hasSoundCloud @20 :Bool;
  hasSinaWeibo  @21 :Bool;
  hasVKontakte  @22 :Bool;

  networks @23 :List(Network);
}

struct CardAnalytics {
  cardBLEDeliveries  @0  :Int64;
  cardLinkDeliveries @1  :Int64;
  cardViews          @2  :Int64;
  cardShares         @3  :Int64;
  facebookViews      @4  :Int64;
  twitterViews       @5  :Int64;
  instagramViews     @6  :Int64;
  snapchatViews      @7  :Int64;
  googlePlusViews    @8  :Int64;
  youTubeViews       @9  :Int64;
  pinterestViews     @10 :Int64;
  tumblrViews        @11 :Int64;
  linkedInViews      @12 :Int64;
  periscopeViews     @13 :Int64;
  vineViews          @14 :Int64;
  soundCloudViews    @15 :Int64;
  sinaWeiboViews     @16 :Int64;
  vKontakteViews     @17 :Int64;
}

struct CreateCardRequest {
  card @0 :Card;
}

struct UpdateCardRequest {
  card @0 :Card;
}

struct CardRequest {
  id      @0 :Int64;
  version @1 :Int32;
}

struct CardResponse {
  status @0 :Status;
  card   @1 :Card;
  analytics @2 :CardAnalytics;

  enum Status {
    current @0;
    updated @1;
    created @2;
  }
}
