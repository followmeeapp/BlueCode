//
//  SectionObject.h
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import <Realm/Realm.h>

@interface SectionObject : RLMObject

@property NSInteger id; // primaryKey

@property NSDate *timestamp;

@property (readonly) NSArray *visibleCards;
@property (readonly) NSArray *hiddenCards;

@property NSData *cards; // Stores a Cap'n Proto object containing visibleCards and hiddenCards

@end

RLM_ARRAY_TYPE(SectionObject)
    // This protocol enables typed collections. i.e.: RLMArray<SectionObject>
