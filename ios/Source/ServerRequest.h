//
//  ServerRequest.h
//  Follow
//
//  Created by Erich Ocean on 7/13/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

@interface ServerRequest : NSObject

+ (NSData *) newHelloRequestWithId: (NSInteger) requestId;

+ (NSData *)
newJoinRequestWithId: (NSInteger)  requestId
telephone:            (NSString *) telephone
digitsId:             (NSString *) digitsId
email:                (NSString *) email
emailIsVerified:      (BOOL)       isVerified;

+ (NSData *)
newCreateCardRequestWithId: (NSInteger)      requestId
properties:                 (NSDictionary *) props;

+ (NSData *)
newUpdateCardRequestWithId: (NSInteger)      requestId
properties:                 (NSDictionary *) props;

+ (NSData *)
newCardRequestWithId: (NSInteger) requestId
cardId:               (NSInteger) cardId
version:              (int)       version;

+ (NSData *)
newCreateBackupRequestWithId: (NSInteger) requestId
timestamp:                    (int64_t)   timestamp
previous:                     (int64_t)   previous;

+ (NSData *) newBackupRequestWithId: (NSInteger) requestId;

@end
