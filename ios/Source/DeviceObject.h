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

@end

RLM_ARRAY_TYPE(DeviceObject)
    // This protocol enables typed collections. i.e.: RLMArray<DeviceObject>
