//
//  UserObject.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import <Realm/Realm.h>

@interface UserObject : RLMObject

@property NSInteger id; // primaryKey

@property NSInteger activeDeviceId;
@property NSData *activeDeviceUUID;

@property NSInteger cardId;

@property NSData *advertisingInfo;

@property int64_t lastBackup;

@end

RLM_ARRAY_TYPE(UserObject)
    // This protocol enables typed collections. i.e.: RLMArray<UserObject>
