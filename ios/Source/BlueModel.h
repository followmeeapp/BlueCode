//
//  BlueModel.h
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

@class UserObject;
@class CardObject;
@class NetworkObject;
@class DeviceObject;
@class DeviceCardObject;
@class SectionObject;
@class CardSectionObject;

@interface BlueModel : NSObject

- (void) destroyRealmAndExit;

- (UserObject *) activeUser;
- (DeviceObject *) activeDevice;
- (CardObject *) activeUserCard;

@end
