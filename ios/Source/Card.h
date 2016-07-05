//
//  Card.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import <Realm/Realm.h>

#import "Network.h"

@interface Card : RLMObject

@property NSInteger id;
@property long long guid;

@property int version;
    // This can be used to refresh a card. All CardDrops should have the same version;
    // if not, they need to be refershed. (We may be able to refersh them without
    // contacting the server given the corresponding Card object.
    //
    // TODO(Erich) Automatically referesh any linked cardDrops when a Card is updated.

// TODO(Erich) Need the other properties that are associated with a Card object.
@property BOOL hasFacebook;
@property BOOL hasTwitter;
@property BOOL hasInstagram;

// TODO(Erich) Need a way to start a conversation with the owner of this card.
// Maybe have the server enable it via an RPC call? Hmm.

@property RLMArray<Network *><Network> *networks;

@property (readonly) RLMLinkingObjects *cardDrops;
    // This is only meant to be applicable to cards owned by the logged-in user.

@end

RLM_ARRAY_TYPE(Card)
    // This protocol enables typed collections. i.e.: RLMArray<Card>
