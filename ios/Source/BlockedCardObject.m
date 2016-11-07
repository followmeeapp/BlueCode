//
//  BlockedCardObject.m
//  Follow
//
//  Created by Erich Ocean on 10/6/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlockedCardObject.h"

@implementation BlockedCardObject

+ (NSString *) primaryKey
{
    return @"id";
}

+ (NSDictionary *) defaultPropertyValues
{
    return @{
        @"timestamp": [NSDate dateWithTimeIntervalSinceNow: 0],
    };
}

@end
