//
//  Network.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import <Realm/Realm.h>

@class Card;

typedef NS_ENUM(NSInteger, NetworkType) {
    FacebookType = 0,
    TwitterType,
    InstagramType
};

@interface Network : RLMObject

@property NSInteger id;

@property NetworkType type;
@property NSString *username;

@property Card *card;

@end

RLM_ARRAY_TYPE(Network)
    // This protocol enables typed collections. i.e.: RLMArray<Network>
