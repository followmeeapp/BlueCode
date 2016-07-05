//
//  CardFollow.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import <Realm/Realm.h>

@class Card;
@class User;

@interface CardFollow : RLMObject

@property NSInteger id;

@property int version;
    // This can be used to refresh a card follow. It should match the version
    // for the Card object this CardDrop is for.
    //
    // TODO(Erich) Whenever a Card is updated, the CardFollow needs to be updated
    // to match.

// TODO(Erich) Need additional "display" properties.
@property NSString *displayName;
@property NSString *avatarURLString;
@property NSString *listAvatarURLString;
@property BOOL hasFacebook;
@property BOOL hasTwitter;
@property BOOL hasInstagram;

@property long long cardGuid;

@property Card *card;
    // When nil, the cardGuid can be used to fetch the Card from the server.
    // TODO(Erich) Ensure that this is always set whenever a Card is inserted.

@property User *user;
    // This is never nil; we only store CardFollows for the logged-in User.

@end

RLM_ARRAY_TYPE(CardFollow)
    // This protocol enables typed collections. i.e.: RLMArray<CardFollow>
