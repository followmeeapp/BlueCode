//
//  CardListBuilder.h
//  Follow
//
//  Created by Erich Ocean on 7/30/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

@interface CardListBuilder : NSObject

+ (NSArray *) visibleCardsFromData: (NSData *) data;
+ (NSArray *) hiddenCardsFromData: (NSData *) data;

@end
