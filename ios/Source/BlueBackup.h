//
//  BlueBackup.h
//  Follow
//
//  Created by Erich Ocean on 10/22/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface BlueBackup : NSObject

- (IBAction) backupNow: sender;

- (IBAction) logout: sender;

- (void)
handleBackupWithTimestamp: (int64_t)  timestamp
data:                      (NSData *) data;

@end
