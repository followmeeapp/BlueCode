//
//  Utilities.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSData (Hex)

- (NSString *) hexString;
+ (NSData *) dataFromHexString: (NSString * ) hexString;
- (NSString *) encodeBase64;

@end

@interface NSDate (Blue)

+ (instancetype) dateWithMillisecondsSince1970: (int64_t) milliseconds;

- (int64_t) millisecondsSince1970;

@end
