//
//  CardListBuilder.h
//  Follow
//
//  Created by Erich Ocean on 7/30/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

@interface CardListBuilder : NSObject

+ (NSData *)
dataWithVisibleCards:  (NSArray *) visibleCards
visibleCardTimestamps: (NSArray *) visibleCardTimestamps
hiddenCards:           (NSArray *) hiddenCards
hiddenCardTimestamps:  (NSArray *) hiddenCardTimestamps;

+ (NSData *)
dataWithVisibleCards: (NSArray *) visibleCards
hiddenCards:          (NSArray *) hiddenCards;

+ (NSArray *) visibleCardsFromData: (NSData *) data;
+ (NSArray *) visibleCardTimestampsFromData: (NSData *) data;
+ (NSArray *) hiddenCardsFromData: (NSData *) data;
+ (NSArray *) hiddenCardTimestampsFromData: (NSData *) data;

@end
