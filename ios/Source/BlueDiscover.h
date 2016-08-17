//
//  BlueDiscover.h
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//
//  Based on code from https://github.com/omergul123/Discovery
//  Apache 2 license.
//

#import <CoreBluetooth/CoreBluetooth.h>
#import <CoreLocation/CoreLocation.h>

#import "BlueDiscoverSession.h"

@interface BlueDiscover : NSObject <CBCentralManagerDelegate, CBPeripheralManagerDelegate, CBPeripheralDelegate>

// disable the default initializer
- (instancetype) init NS_UNAVAILABLE;

/**
 * Initialize the Discovery object with a UUID specific to your app.
 */
- (instancetype) initWithUUID: (CBUUID *) uuid;

/**
 * Username specific to your device.
 **/
- (void) startAdvertisingWithUsername: (NSString *) username;

/**
 * Stop advertising.
 */
- (void) stopAdvertising;

/**
 * The usersBlock is triggered periodically in order of users' proximity.
 **/
- (void) startDiscovering: (void (^)(NSArray *users, BOOL usersChanged)) sessionsBlock;

/**
 * Stop discovering.
 **/
- (void) stopDiscovering;

/**
 * Returns the user user from our user dictionary according to its peripheralId.
 */
- (BlueDiscoverSession *) sessionWithPeripheralId: (NSString *) peripheralId;

/**
 * Changing these properties will start/stop advertising/discovery
 */
@property (nonatomic, getter=isAdvertising) BOOL shouldAdvertise;
@property (nonatomic, getter=isDiscovering) BOOL shouldDiscover;

/**
 * UUID is used for id as advertisement, peripheral services and characteristics.
 * It should be unique to you app, not to your device. Otherwise the peers won't be able to discover each other.
 */
@property (strong, nonatomic, readonly) CBUUID *uuid;

/**
 * This is the value that should be unique to your device.
 * The users will be identified with username. This value is advertised as the user ID.
 */
@property (strong, nonatomic, readonly) NSString *username;

/**
 * Central and Peripheral managers and the queue for the manager events.
 */
@property (nonatomic) dispatch_queue_t queue;
@property (strong, nonatomic) CBCentralManager *centralManager;
@property (strong, nonatomic) CBPeripheralManager *peripheralManager;

/*
 * This dictionary holds all the discovered devices related to our UUID.
 * However it also contains the users that are NOT already identified.
 * We use periperal ID's as the keys.
 */
@property (strong, nonatomic, readonly) NSMutableDictionary *sessionsMap;

/*
 * Discovery removes the users if can not re-see them after some amount of time, assuming the device-user is gone.
 * The default value is 3 seconds. You can set your own values.
 */
@property (nonatomic) NSTimeInterval userTimeoutInterval;

/*
 * Update interval is the interval that your usersBlock gets triggered.
 */
@property (nonatomic) NSTimeInterval updateInterval;

@end
