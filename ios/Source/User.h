//
//  User.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import <Realm/Realm.h>

#import "Card.h"
#import "CardDrop.h"
#import "CardFollow.h"
#import "Conversation.h"

@interface User : RLMObject

@property NSInteger id;
@property long long guid;

@property NSString *telephone;
@property NSString *digitsId;
@property NSString *layerId;

@property RLMArray<Card *><Card> *cards;
@property RLMArray<CardDrop *><CardDrop> *cardDrops;
@property RLMArray<CardFollow *><CardFollow> *cardFollows;
@property RLMArray<Conversation *><Conversation> *conversations;

@end

// This protocol enables typed collections. i.e.:
// RLMArray<User>
RLM_ARRAY_TYPE(User)
