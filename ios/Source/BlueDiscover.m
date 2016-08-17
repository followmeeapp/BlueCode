//
//  BlueDiscover.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueDiscover.h"

#import "BlueApp.h"

@interface BlueDiscover ()

@property (nonatomic, copy) void (^sessionsBlock)(NSArray *users, BOOL usersChanged);
@property (strong, nonatomic) NSTimer *timer;
@property (nonatomic, getter=isInBackgroundMode) BOOL inBackgroundMode;

@end

static double bgStartTime = 0.0f;

@implementation BlueDiscover

- (instancetype) initWithUUID: (CBUUID *) uuid
{
    if (self = [super init]) {
        _uuid = uuid;

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

- (void) startAdvertisingWithUsername: (NSString *) username
{
    _username = username;
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
        }

    } else {
        if (self.peripheralManager) {
            [self.peripheralManager stopAdvertising];
            self.peripheralManager.delegate = nil;
            self.peripheralManager = nil;
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
    [self startTimer];
}

- (void) startAdvertising
{
    NSDictionary *advertisingData = @{
        CBAdvertisementDataLocalNameKey: self.username,
        CBAdvertisementDataServiceUUIDsKey: @[self.uuid]
    };

    // create our characteristics
    CBMutableCharacteristic *characteristic = [[CBMutableCharacteristic alloc] initWithType: self.uuid
                                                                               properties:   CBCharacteristicPropertyRead
                                                                               value:        [self.username dataUsingEncoding: NSUTF8StringEncoding]
                                                                               permissions:  CBAttributePermissionsReadable                        ];

    // create the service with the characteristics
    CBMutableService *service = [[CBMutableService alloc] initWithType: self.uuid primary: YES];
    service.characteristics = @[characteristic];
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

}

- (void) peripheralManagerDidUpdateState: (CBPeripheralManager *) peripheral
{
    if (peripheral.state == CBPeripheralManagerStatePoweredOn) {
        [self startAdvertising];

    } else {
        NSLog(@"Peripheral manager state: %ld", peripheral.state);
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
        NSLog(@"Central manager state: %ld", central.state);
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
        self.sessionsBlock([sessions mutableCopy], sessionsChanged);
    }
}

- (void) checkList
{
    double currentTime = [[NSDate date] timeIntervalSince1970];

    NSMutableArray *discardedKeys = [NSMutableArray array];

    for (NSString *key in self.sessionsMap) {
        BlueDiscoverSession *session = [self.sessionsMap objectForKey: key];

        NSTimeInterval diff = currentTime - session.updateTime;

        // We remove the session if we haven't seen it for the userTimeInterval amount of seconds.
        // You can simply set the userTimeInterval variable anything you want.
        if (diff > self.userTimeoutInterval) {
            [discardedKeys addObject: key];
        }
    }

    // update the list if we removed a session.
    if (discardedKeys.count > 0) {
        [self.sessionsMap removeObjectsForKeys: discardedKeys];
        [self updateList];

    } else {
        // simply update the list, because the order of the users may have changed.
        [self updateList: NO];
    }
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
        [[NSUserDefaults standardUserDefaults] synchronize];
        NSLog(@"Bgtime : %f", bgTime);
    }

    BlueDiscoverSession *session = [self sessionWithPeripheralId: peripheral.identifier.UUIDString];

    if (session == nil) {
        NSLog(@"Adding session: %@", username);
        session = [[BlueDiscoverSession alloc] initWithPerpipheral: peripheral];
        session.username = nil;

        session.identified = NO;
        session.peripheral.delegate = self;

        [self.sessionsMap setObject: session forKey: session.peripheralId];
    }

    if (!session.isIdentified) {
        // We check if we can get the username from the advertisement data,
        // in case the advertising peer application is working at foreground
        // if we get the name from advertisement we don't have to establish a peripheral connection
        if (username != (id)[NSNull null] && username.length > 0 ) {
            session.username = username;
            session.identified = YES;

            // we update our list for callback block
            [self updateList];

        } else {
            // nope we could not get the username from CBAdvertisementDataLocalNameKey,
            // we have to connect to the peripheral and try to get the characteristic data
            // add we will extract the username from characteristics.

            if (peripheral.state == CBPeripheralStateDisconnected) {
                [self.centralManager connectPeripheral: peripheral options: nil];
            }
        }
    }

    // update the rss and update time
    session.rssi = [RSSI floatValue];
    session.updateTime = [[NSDate date] timeIntervalSince1970];
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
    // loop the services
    // since we are looking forn only one service, services array probably contains only one or zero item
    for (CBService *service in peripheral.services) {
        [peripheral discoverCharacteristics: nil forService: service];
    }
}

- (void)
peripheral:                           (CBPeripheral *) peripheral
didDiscoverCharacteristicsForService: (CBService *)    service
error:                                (NSError *)      error
{
    BlueDiscoverSession *session = [self sessionWithPeripheralId: peripheral.identifier.UUIDString];
    NSLog(@"Did discover characteristics of: %@ - %@", session.username, service.characteristics);

    if (!error) {
        // loop through to find our characteristic
        for (CBCharacteristic *characteristic in service.characteristics) {
            if ([characteristic.UUID isEqual: self.uuid]) {
                [peripheral readValueForCharacteristic: characteristic];
                //[peripheral setNotifyValue:YES forCharacteristic:characteristic];
            }
        }
    }

}

- (void)
peripheral:                      (CBPeripheral *)     peripheral
didUpdateValueForCharacteristic: (CBCharacteristic *) characteristic
error:                           (NSError *)          error
{
    NSString *valueStr = [[NSString alloc] initWithData: characteristic.value encoding: NSUTF8StringEncoding];
    NSLog(@"CBCharacteristic updated value: %@", valueStr);

    // if the value is not nil, we found our username!
    if (valueStr != nil) {
        BlueDiscoverSession *session = [self sessionWithPeripheralId: peripheral.identifier.UUIDString];

        session.username = valueStr;
        session.identified = YES;

        [self updateList];

        // cancel the subscription to our characteristic
        [peripheral setNotifyValue: NO forCharacteristic: characteristic];

        // and disconnect from the peripehral
        [self.centralManager cancelPeripheralConnection: peripheral];
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
