//
//  AnalyticsObject.h
//  Follow
//
//  Created by Erich Ocean on 10/6/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import <Realm/Realm.h>

@interface AnalyticsObject : RLMObject

@property int64_t timestamp;

@property NSData *action;

@end
