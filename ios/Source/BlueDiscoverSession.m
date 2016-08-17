//
//  BlueDiscoverSession.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueDiscoverSession.h"

@interface BlueDiscoverSession ()

@property (strong, nonatomic) EasedValue *easedProximity;

@end

@implementation BlueDiscoverSession

- (instancetype) initWithPerpipheral: (CBPeripheral *) peripheral
{
    if (self = [super init]) {
        _peripheral = peripheral;
        _peripheralId = peripheral.identifier.UUIDString;
        _easedProximity = [[EasedValue alloc] init];
    }

    return self;
}

- (void) setRssi: (float) rssi
{
    _rssi = rssi;
    _proximity = [self convertRSSItoProximity: rssi];
}

- (NSInteger) convertRSSItoProximity: (NSInteger) rssi
{
    // eased value doesn't support negative values
    self.easedProximity.value = labs(rssi);
    [self.easedProximity update];

    NSInteger proximity = self.easedProximity.value * -1.0f;

    return proximity;
}

@end
