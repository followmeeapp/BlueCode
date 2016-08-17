//
//  ServerResponse.h
//  Follow
//
//  Created by Erich Ocean on 7/13/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

@class BlueModel;

@interface ServerResponse : NSObject

+ (void) handleResponse: (NSData *) data;

@end
