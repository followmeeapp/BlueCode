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

@property BOOL isBLECard;
@property BOOL isFromBlueCardLink;
@property BOOL isFromBackup;

@property NSInteger visibleSection; // This is the first (oldest) section the card is a member of.

@property NSString *fullName;
@property NSString *location;
@property NSString *bio;

@property NSString *status;
@property NSDate *statusUpdatedAt;

@property NSString *avatarURLString;
@property NSString *backgroundURLString;

@property double backgroundOpacity;
@property NSInteger rowOffset;

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

// Card Analytics
@property NSInteger cardBLEDeliveries;
@property NSInteger cardLinkDeliveries;
@property NSInteger cardViews;
@property NSInteger cardShares;
@property NSInteger facebookViews;
@property NSInteger twitterViews;
@property NSInteger instagramViews;
@property NSInteger snapchatViews;
@property NSInteger googlePlusViews;
@property NSInteger youTubeViews;
@property NSInteger pinterestViews;
@property NSInteger tumblrViews;
@property NSInteger linkedInViews;
@property NSInteger periscopeViews;
@property NSInteger vineViews;
@property NSInteger soundCloudViews;
@property NSInteger sinaWeiboViews;
@property NSInteger vKontakteViews;

@property (readonly) NSArray *sections;
@property NSData *sectionData; // Stores a Cap'n Proto object containing sections

@property RLMArray<NetworkObject *><NetworkObject> *networks;
    // There should only be at most one network of each Network type.

@end

RLM_ARRAY_TYPE(CardObject)
    // This protocol enables typed collections. i.e.: RLMArray<CardObject>
