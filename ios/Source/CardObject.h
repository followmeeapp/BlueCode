//
//  CardObject.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import <Realm/Realm.h>

#import "NetworkObject.h"

@interface CardObject : RLMObject

@property NSInteger id; // primaryKey

@property int version;
    // This can be used to refresh a card.

@property NSDate *timestamp;

@property BOOL isOwnCard;
@property NSInteger visibleSection;

@property NSString *fullName;
@property NSString *location;
@property NSString *bio;

@property NSString *avatarURLString;
@property NSString *backgroundURLString;

@property BOOL hasPokemonGo;
@property BOOL hasFacebook;
@property BOOL hasTwitter;
@property BOOL hasInstagram;
@property BOOL hasSnapchat;
@property BOOL hasGooglePlus;
@property BOOL hasYouTube;
@property BOOL hasPinterest;
@property BOOL hasTumblr;
@property BOOL hasLinkedIn;
@property BOOL hasPeriscope;
@property BOOL hasVine;
@property BOOL hasSoundCloud;
@property BOOL hasSinaWeibo;
@property BOOL hasVKontakte;

@property RLMArray<NetworkObject *><NetworkObject> *networks;
    // There should only be at most one network of each Network type.

@end

RLM_ARRAY_TYPE(CardObject)
    // This protocol enables typed collections. i.e.: RLMArray<CardObject>
