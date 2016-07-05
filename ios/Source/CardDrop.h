//
//  CardDrop.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import <Realm/Realm.h>

@class Card;
@class User;

@interface CardDrop : RLMObject

@property NSInteger id;
@property long long guid;

@property int version;
    // This can be used to refresh a card drop. It should match the version
    // for the Card object this CardDrop is for.

@property NSDate *expires;

@property float latitude;
@property float longitude;

// TODO(Erich) Need additional "display" properties.
@property NSString *displayName;
@property NSString *avatarURLString;
@property NSString *listAvatarURLString;
@property BOOL hasFacebook;
@property BOOL hasTwitter;
@property BOOL hasInstagram;

@property long long cardGuid;

@property User *user;
    // This is typically nil, unless the card drop was mady be the logged-in user.
    // TODO(Erich) Ensure that this is always set whenever a CardDrop created by the user is inserted.

@property Card *card;
    // When nil, the cardGuid can be used to fetch the Card from the server.
    // TODO(Erich) Ensure that this is always set whenever a Card is inserted.

@end

RLM_ARRAY_TYPE(CardDrop)
    // This protocol enables typed collections. i.e.: RLMArray<CardDrop>
