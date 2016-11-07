//
//  DeviceObject.h
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import <Realm/Realm.h>

@interface DeviceObject : RLMObject

@property NSInteger id; // primaryKey

@property NSDate *lastSeen;

@property BOOL needsToBeCreated;

@property (readonly) NSArray *visibleCards;
@property (readonly) NSArray *hiddenCards;

@property NSData *cardData; // Stores a Cap'n Proto object containing visibleCards and hiddenCards

@property int64_t lastAnalytics;

// This expects the cards to be in ascending order by timestamp, i.e.
// 0 is the oldest card, N is the mostly recently added card.
// THIS IS THE OPPOSITE ORDER THAT visibleCards IS PROVIDED IN.
- (void)
updateVisibleCards: (NSArray *) visibleCards
hiddenCards:        (NSArray *) hiddenCards;

@end

RLM_ARRAY_TYPE(DeviceObject)
    // This protocol enables typed collections. i.e.: RLMArray<DeviceObject>
