//
//  BlueDiscover.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueDiscover.h"

#import "BlueApp.h"

#import "CardObject.h"
#import "DeviceObject.h"

@interface BlueDiscover ()

@property (nonatomic, copy) void (^sessionsBlock)(NSArray *users, BOOL usersChanged);
@property (strong, nonatomic) NSTimer *timer;
@property (nonatomic, getter=isInBackgroundMode) BOOL inBackgroundMode;

@property (nonatomic, strong) CBUUID *fullNameUUID;
@property (nonatomic, strong) CBUUID *locationUUID;

@end

static double bgStartTime = 0.0f;

NSString *uuidFullNameStr = @"82635DDC-137A-4858-A7B4-7AC9E45B4ACC";
NSString *uuidLocationStr = @"8FEFD378-EB61-435B-A315-1A8A79930988";

@implementation BlueDiscover

- (instancetype) initWithUUID: (CBUUID *) uuid
{
    if (self = [super init]) {
        self.fullNameUUID = [CBUUID UUIDWithString: uuidFullNameStr];
        self.locationUUID = [CBUUID UUIDWithString: uuidLocationStr];

        self.uuid = uuid;

        _inBackgroundMode = NO;

        _userTimeoutInterval = 3;
        _updateInterval = 2;

        [[NSNotificationCenter defaultCenter] addObserver: self
                                              selector:    @selector(appDidEnterBackground:)
                                              name:        UIApplicationDidEnterBackgroundNotification
                                              object:      nil                                        ];

        [[NSNotificationCenter defaultCenter] addObserver: self
                                              selector:    @selector(appWillEnterForeground:)
                                              name:        UIApplicationWillEnterForegroundNotification
                                              object:      nil                                         ];


        // we will hold the detected users here
        _sessionsMap = [NSMutableDictionary dictionary];

        // start the central and peripheral managers
        _queue = dispatch_queue_create("social.blue.discovery", DISPATCH_QUEUE_SERIAL);

        _shouldAdvertise = NO;
        _shouldDiscover = NO;
    }

    return self;
}

- (void)
startAdvertisingWithDeviceId: (NSInteger)    deviceId
card:                         (CardObject *) card
{
    NSString *json = [NSString stringWithFormat: @"{\"d\":%@,\"c\":%@,\"v\":%@}", @(deviceId), @(card.id), @(card.version)];
    NSLog(@"advertising username = %@", json);

    self.username = json;
    self.fullName = card.fullName;
    self.location = card.location;

    if (self.location == nil) self.location = @""; // Always need to have this.

    self.shouldAdvertise = YES;
}

- (void) stopAdvertising
{
    self.shouldAdvertise = NO;
}

- (void) startDiscovering: (void (^)(NSArray *users, BOOL usersChanged)) sessionsBlock
{
    self.sessionsBlock = sessionsBlock;
    self.shouldDiscover = YES;
}

- (void) stopDiscovering
{
    self.shouldDiscover = NO;
}

-(void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self name: UIApplicationWillEnterForegroundNotification object: nil];
    [[NSNotificationCenter defaultCenter] removeObserver: self name: UIApplicationDidEnterBackgroundNotification object: nil];

    NSLog(@"Discovery deallocated.");
}

-(void) setShouldAdvertise: (BOOL) shouldAdvertise
{
    if (_shouldAdvertise == shouldAdvertise) return;

    _shouldAdvertise = shouldAdvertise;

    if (shouldAdvertise) {
        if (!self.peripheralManager) {
            self.peripheralManager = [[CBPeripheralManager alloc] initWithDelegate: self queue: self.queue];

        } else if (self.peripheralManager.state == CBPeripheralManagerStatePoweredOn) {
            [self startAdvertising];
        }

    } else {
        if (self.peripheralManager) {
            [self.peripheralManager stopAdvertising];
        }
    }
}

- (void) setShouldDiscover: (BOOL) shouldDiscover
{
    if (_shouldDiscover == shouldDiscover) return;

    _shouldDiscover = shouldDiscover;

    if (shouldDiscover) {
        if (!self.centralManager) {
            self.centralManager = [[CBCentralManager alloc] initWithDelegate: self queue: self.queue];
        }

        if (!self.timer) [self startTimer];

    } else {
        if (self.centralManager) {
            [self.centralManager stopScan];
            self.centralManager.delegate = nil;
            self.centralManager = nil;
        }

        if (self.timer) [self stopTimer];
    }
}

- (void) startTimer
{
    self.timer = [NSTimer scheduledTimerWithTimeInterval: self.updateInterval
                          target:                         self
                          selector:                       @selector(checkList)
                          userInfo:                       nil
                          repeats:                        YES                ];
}

- (void) stopTimer
{
    [self.timer invalidate];
    self.timer = nil;
}

- (void) setUpdateInterval: (NSTimeInterval) updateInterval
{
    _updateInterval = updateInterval;

    // restart the timers
    [self stopTimer];
    [self startTimer];
}

- (void) appDidEnterBackground: (NSNotification *) notification
{
    self.inBackgroundMode = YES;
    bgStartTime = CFAbsoluteTimeGetCurrent();
    [self stopTimer];
}

- (void) appWillEnterForeground: (NSNotification *) notification
{
    self.inBackgroundMode = NO;

    [[NSUserDefaults standardUserDefaults] synchronize];

    [self startTimer];
}

- (void) startAdvertising
{
    NSDictionary *advertisingData = @{
        CBAdvertisementDataLocalNameKey: self.username,
        CBAdvertisementDataServiceUUIDsKey: @[self.uuid]
    };

    // create our characteristics
    CBMutableCharacteristic *usernameCharacteristic = [[CBMutableCharacteristic alloc] initWithType: self.uuid
                                                                                       properties:   CBCharacteristicPropertyRead
                                                                                       value:        [self.username dataUsingEncoding: NSUTF8StringEncoding]
                                                                                       permissions:  CBAttributePermissionsReadable                        ];

    CBMutableCharacteristic *fullNameCharacteristic = [[CBMutableCharacteristic alloc] initWithType: self.fullNameUUID
                                                                                       properties:   CBCharacteristicPropertyRead
                                                                                       value:        [self.fullName dataUsingEncoding: NSUTF8StringEncoding]
                                                                                       permissions:  CBAttributePermissionsReadable                        ];

    CBMutableCharacteristic *locationCharacteristic = [[CBMutableCharacteristic alloc] initWithType: self.locationUUID
                                                                                       properties:   CBCharacteristicPropertyRead
                                                                                       value:        [self.location dataUsingEncoding: NSUTF8StringEncoding]
                                                                                       permissions:  CBAttributePermissionsReadable                        ];

    // create the service with the characteristics
    CBMutableService *service = [[CBMutableService alloc] initWithType: self.uuid primary: YES];
    service.characteristics = @[usernameCharacteristic, fullNameCharacteristic, locationCharacteristic];
    [self.peripheralManager addService: service];

    [self.peripheralManager startAdvertising: advertisingData];
}

- (void) startDetecting
{

    NSDictionary *scanOptions = @{ CBCentralManagerScanOptionAllowDuplicatesKey: @(YES) };
    NSArray *services = @[self.uuid];

    // we only listen to the service that belongs to our uuid
    // this is important for performance and battery consumption
    [self.centralManager scanForPeripheralsWithServices: services options: scanOptions];
}

- (void)
peripheralManagerDidStartAdvertising: (CBPeripheralManager *) peripheral
error:                                (NSError *)             error
{
    NSLog(@"-[BlueDiscover peripheralManagerDidStartAdvertising:error:] %@", error);
}

- (void) peripheralManagerDidUpdateState: (CBPeripheralManager *) peripheral
{
    if (peripheral.state == CBPeripheralManagerStatePoweredOn) {
        [self startAdvertising];

    } else {
        NSLog(@"Peripheral manager state: %ld", (long)peripheral.state);
    }
}

- (void) centralManagerDidUpdateState: (CBCentralManager *) central
{
    if (central.state == CBCentralManagerStatePoweredOn) {
        [self startDetecting];

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [APP_DELEGATE hideBluetoothMonitor];
        });

    } else {
        NSLog(@"Central manager state: %ld", (long)central.state);
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [APP_DELEGATE showBluetoothMonitor];
        });
    }
}

- (void) updateList
{
    [self updateList: YES];
}

- (void) updateList: (BOOL) sessionsChanged
{
    NSMutableArray *sessions;

    @synchronized(self.sessionsMap) {
        sessions = [[[self sessionsMap] allValues] mutableCopy];
    }

    // remove unidentified sessions
    NSMutableArray *discardedItems = [NSMutableArray array];

    for (BlueDiscoverSession *session in sessions) {
        if (!session.isIdentified) [discardedItems addObject: session];
    }

    [sessions removeObjectsInArray: discardedItems];

    // we sort the list according to "proximity".
    // so the client will receive ordered users according to the proximity.
    [sessions sortUsingDescriptors: [NSArray arrayWithObjects: [NSSortDescriptor sortDescriptorWithKey: @"proximity" ascending: NO], nil]];

    if (self.sessionsBlock) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
            self.sessionsBlock([sessions mutableCopy], sessionsChanged);
        });
    }
}

- (void) checkList
{
//    double currentTime = [[NSDate date] timeIntervalSince1970];
//
//    NSMutableArray *discardedKeys = [NSMutableArray array];
//
//    for (NSString *key in self.sessionsMap) {
//        BlueDiscoverSession *session = [self.sessionsMap objectForKey: key];
//
//        NSTimeInterval diff = currentTime - session.updateTime;
//
//        // We remove the session if we haven't seen it for the userTimeInterval amount of seconds.
//        // You can simply set the userTimeInterval variable anything you want.
//        if (diff > self.userTimeoutInterval) {
//            [discardedKeys addObject: key];
//        }
//    }
//
//    if (discardedKeys.count > 0) {
//        [self.sessionsMap removeObjectsForKeys: discardedKeys];
//    }
}

- (BlueDiscoverSession *) sessionWithPeripheralId: (NSString *) peripheralId
{
    return [self.sessionsMap valueForKey: peripheralId];
}

#pragma mark - CBCentralManagerDelegate

- (void)
centralManager:        (CBCentralManager *) central
didDiscoverPeripheral: (CBPeripheral *)     peripheral
advertisementData:     (NSDictionary *)     advertisementData
RSSI:                  (NSNumber *)         RSSI
{
    NSString *username = advertisementData[CBAdvertisementDataLocalNameKey];
//    NSLog(@"Discovered: %@ %@ at %@ -- %@", peripheral.name, peripheral.identifier, RSSI, username);

    if (self.isInBackgroundMode) {
        double bgTime = (CFAbsoluteTimeGetCurrent() - bgStartTime);
        [[NSUserDefaults standardUserDefaults] setDouble: bgTime forKey: @"bgTime"];
//        NSLog(@"Bgtime : %f", bgTime);
    }

    BlueDiscoverSession *session = [self sessionWithPeripheralId: peripheral.identifier.UUIDString];

    if (session == nil) {
        NSLog(@"Adding session: %@", username);
        session = [[BlueDiscoverSession alloc] initWithPerpipheral: peripheral];
        session.username = nil;

        session.identified = NO;
        session.finished = NO;
        session.peripheral.delegate = self;

        [self.sessionsMap setObject: session forKey: session.peripheralId];
    }

    // update the rss and update time
    session.rssi = [RSSI floatValue];
    session.updateTime = [[NSDate date] timeIntervalSince1970];

    if (!session.isIdentified) {
        NSData *jsonData = nil;
        if (username == nil || username == (id)[NSNull null] || username.length == 0) {
            // Note: Purposefully not valid JSON.
            jsonData = [@"{" dataUsingEncoding: NSUTF8StringEncoding];

        } else {
            // *Might* be valid JSON.
            jsonData = [username dataUsingEncoding: NSUTF8StringEncoding];
        }

        NSError *error = nil;
        NSDictionary *dict = [NSJSONSerialization JSONObjectWithData: jsonData
                                                  options:            0
                                                  error:              &error  ];

        if (error) {
            // Nope we could not get valid JSON from CBAdvertisementDataLocalNameKey,
            // we have to connect to the peripheral and try to get the characteristic data
            // add we will extract the username from characteristics.
            if (peripheral.state == CBPeripheralStateDisconnected) {
                [self.centralManager connectPeripheral: peripheral options: nil];
            }

        } else {
            // We got valid JSON from the advertisement data (peer application is working at foreground)
            // and there were no other issues (e.g. ad data cut off).

            session.username = username;
            session.identified = YES;

            RLMRealm *realm = [RLMRealm defaultRealm];
            [realm beginWriteTransaction];

            // We need to create/update the DeviceObject.
            DeviceObject *device = [DeviceObject objectForPrimaryKey: dict[@"d"]];
            if (!device) {
                device = [[DeviceObject alloc] init];
                device.id = [dict[@"d"] integerValue];
            }

            device.lastSeen = [NSDate dateWithTimeIntervalSinceNow: 0];

            [realm addOrUpdateObject: device];

//            BOOL shouldNotify = NO;

            // Do we need to gather the rest of the card's BLE properties?
            CardObject *card = [CardObject objectForPrimaryKey: dict[@"c"]];
            if (!card || card.version < [dict[@"v"] intValue]) {
                // We still need to get the fullName and location before we notify BlueApp.
                if (peripheral.state == CBPeripheralStateDisconnected) {
                    [self.centralManager connectPeripheral: peripheral options: nil];
                }

            } else {
                session.finished = YES;

                // If the last time we saw the card was over 90 minutes, then update its timestamp
                // and notify the main controller that it should be moved to the top.
//                NSDate *now = [NSDate dateWithTimeIntervalSinceNow: 0];
//                if (([now timeIntervalSince1970] - [card.timestamp timeIntervalSince1970]) > (60.0 * 90.0)) {
//                    shouldNotify = YES;
//                    card.timestamp = now;
//                    [realm addOrUpdateObject: card];
//
//                } else {
                    [self updateList];
//                }
            }

            [realm commitWriteTransaction];

//            if (shouldNotify) {
//                NSNumber *cardId = @(card.id);
//                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
//                    [[NSNotificationCenter defaultCenter] postNotificationName: @"MoveCardToFront"
//                                                          object:               nil
//                                                          userInfo:             @{ @"cardId": cardId }];
//                });
//            }
        }
    }
}

- (void)
centralManager:             (CBCentralManager *) central
didFailToConnectPeripheral: (CBPeripheral *)     peripheral
error:                      (NSError *)          error
{
    NSLog(@"Peripheral connection failure: %@. (%@)", peripheral, [error localizedDescription]);
}

- (void)
centralManager:       (CBCentralManager *) central
didConnectPeripheral: (CBPeripheral *)     peripheral
{
    BlueDiscoverSession *session = [self sessionWithPeripheralId: peripheral.identifier.UUIDString];
    NSLog(@"Peripheral Connected: %@", session);

    // Search only for services that match our UUID
    // the connection does not guarantee that we will discover the services.
    // if the device is too far away, it may not be possible to discover the service we want.
    [peripheral discoverServices: @[self.uuid]];
}

#pragma mark - CBPeripheralDelegate

- (void)
peripheral:          (CBPeripheral *) peripheral
didDiscoverServices: (NSError *)      error
{
    if (error) return;

    BlueDiscoverSession *session = [self sessionWithPeripheralId: peripheral.identifier.UUIDString];
    NSLog(@"Did discover services of: %@", session.username);

    // Services array probably contains only one or zero item since we are looking for only one service.
    for (CBService *service in peripheral.services) {
        // Only ask for the keys we are missing.
        NSMutableArray *ary = [NSMutableArray array];
        if (session.username == nil) [ary addObject: self.uuid];
        if (session.fullName == nil) [ary addObject: self.fullNameUUID];
        if (session.location == nil) [ary addObject: self.locationUUID];

        if (ary.count == 0) return;

        [peripheral discoverCharacteristics: nil forService: service];
    }
}

- (void)
peripheral:                           (CBPeripheral *) peripheral
didDiscoverCharacteristicsForService: (CBService *)    service
error:                                (NSError *)      error
{
    if (error) return;

    BlueDiscoverSession *session = [self sessionWithPeripheralId: peripheral.identifier.UUIDString];
    NSLog(@"Did discover characteristics of: %@ - %@", session.username, service.characteristics);

    // Read our missing characteristics.
    for (CBCharacteristic *characteristic in service.characteristics) {
        if (session.username == nil && [characteristic.UUID isEqual: self.uuid]) {
            [peripheral readValueForCharacteristic: characteristic];
            //[peripheral setNotifyValue:YES forCharacteristic:characteristic];

        } else if (session.fullName == nil && [characteristic.UUID isEqual: self.fullNameUUID]) {
            [peripheral readValueForCharacteristic: characteristic];

        } else if (session.location == nil && [characteristic.UUID isEqual: self.locationUUID]) {
            [peripheral readValueForCharacteristic: characteristic];
        }
    }
}

- (void)
peripheral:                      (CBPeripheral *)     peripheral
didUpdateValueForCharacteristic: (CBCharacteristic *) characteristic
error:                           (NSError *)          error
{
    if (error) return;

    BlueDiscoverSession *session = [self sessionWithPeripheralId: peripheral.identifier.UUIDString];
    NSString *valueStr = [[NSString alloc] initWithData: characteristic.value encoding: NSUTF8StringEncoding];
    NSLog(@"CBCharacteristic updated value: %@", valueStr);

    if (valueStr == nil) return;

    if (session.username == nil && [characteristic.UUID isEqual: self.uuid]) {
        session.username = valueStr;
        session.identified = YES;

    } else if (session.fullName == nil && [characteristic.UUID isEqual: self.fullNameUUID]) {
        session.fullName = valueStr;

    } else if (session.location == nil && [characteristic.UUID isEqual: self.locationUUID]) {
        session.location = valueStr;
    }

    if (session.fullName && session.location) {
        session.finished = YES;
    }

    // Cancel the subscription to the characteristic.
    [peripheral setNotifyValue: NO forCharacteristic: characteristic];

    if (session.isIdentified && session.isFinished) {
        // Disconnect from the peripheral.
        [self.centralManager cancelPeripheralConnection: peripheral];

        // Notify BlueApp we found a new or updated card.
        [self updateList];
    }
}

- (void)
peripheral:                                  (CBPeripheral *)     peripheral
didUpdateNotificationStateForCharacteristic: (CBCharacteristic *) characteristic
error:                                       (NSError *)          error
{
    NSLog(@"Characteristic Update Notification: %@", error);
}

@end
