//
//  Utilities.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

@interface NSData (Hex)

- (NSString *) hexString;
+ (NSData *) dataFromHexString: (NSString * ) hexString;
- (NSString *) encodeBase64;

@end