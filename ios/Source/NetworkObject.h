//
//  NetworkObject.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import <Realm/Realm.h>

@class CardObject;

typedef NS_ENUM(NSInteger, NetworkType) {
    FacebookType = 0,
    TwitterType,
    InstagramType,
    PokemonGoType,
    SnapchatType,
    GooglePlusType,
    YouTubeType,
    PinterestType,
    TumblrType,
    LinkedInType,
    PeriscopeType,
    VineType,
    SoundCloudType,
    SinaWeiboType,
    VKontakteType,
};

@interface NetworkObject : RLMObject

@property NSString *guid;
    // Should be a UUID string; only used locally, not sent/received from server.

@property NetworkType type;
@property NSString *username;

@property NSInteger cardId;

@end

RLM_ARRAY_TYPE(NetworkObject)
    // This protocol enables typed collections. i.e.: RLMArray<NetworkObject>
