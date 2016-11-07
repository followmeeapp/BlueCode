//
//  BlueDiscoverSession.h
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import <CoreBluetooth/CoreBluetooth.h>
#import <CoreLocation/CoreLocation.h>

#import "EasedValue.h"

@interface BlueDiscoverSession : NSObject

- (instancetype) initWithPerpipheral: (CBPeripheral *) peripheral;

@property (strong, nonatomic, readonly) CBPeripheral *peripheral;
@property (strong, nonatomic, readonly) NSString *peripheralId;

// the unique id for our user.
@property (strong, nonatomic) NSString *username;
@property (strong, nonatomic) NSString *fullName;
@property (strong, nonatomic) NSString *location;

// indicates wheather the user's username is extracted from the peer device.
@property (nonatomic, getter=isIdentified) BOOL identified;
@property (nonatomic, getter=isFinished) BOOL finished;

// rssi
@property (nonatomic) float rssi;

// proximity calculated by EasedValue class.
@property (nonatomic, readonly) NSInteger proximity;

// the last seen time of the device
@property (nonatomic) double updateTime;

@end
