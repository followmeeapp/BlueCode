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
telephone:            (NSString *) telephone;

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

@end
