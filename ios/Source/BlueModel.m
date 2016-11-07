//
//  BlueModel.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueModel.h"

#import <Realm/Realm.h>

#import "UserObject.h"
#import "DeviceObject.h"
#import "CardObject.h"

@implementation BlueModel

- (instancetype) init
{
    if (self = [super init]) {
        [self configureRealm: nil];
    }

    return self;
}

// See https://developer.apple.com/library/ios/technotes/tn2406/_index.html
- (NSURL *) applicationDocumentsDirectory
{
    return [[[NSFileManager defaultManager] URLsForDirectory: NSDocumentDirectory inDomains: NSUserDomainMask] lastObject];
}

- (NSURL *) realmFileURL
{
    NSURL *documentsDirectory = [self applicationDocumentsDirectory];
    NSURL *realmFileURL = [documentsDirectory URLByAppendingPathComponent: @"Blue5.realm"];

    return realmFileURL;
}

- (void) destroyRealmAndExit
{
    NSURL *realmFileURL = [self realmFileURL];

    [[NSFileManager defaultManager] removeItemAtURL: realmFileURL error: NULL];

    exit(0);
}

- (void) configureRealm: (UIApplication *) application
{
    RLMRealmConfiguration *config = [RLMRealmConfiguration defaultConfiguration];

    // Set either the path for the file or the identifer for an in-memory Realm
    // By default config.path will be the Default realm path
    config.fileURL = [self realmFileURL];
    // config.inMemoryIdentifier = @"MyInMemoryRealm";

    // Encryption keys, schema versions and migration blocks are now all set on the
    // config object rather than registered for a path:
//    config.encryptionKey = GetKeyFromKeychain();
//    config.schemaVersion = 10;
//    config.migrationBlock = ^(RLMMigration *migration, uint64_t oldSchemaVersion) {
//        // do stuff
//    };

    // New feature: a Realm configuration can explicitly list which object classes
    // should be stored in the Realm, rather than always including every `RLMObject`
    // and `Object` subclass.
//    config.objectClasses = @[Dog.class, Person.class];

    // Either use the configuration to open a Realm:
//    RLMRealm *realm = [RLMRealm realmWithConfiguration:config error:nil];

    // Or set the configuration used for the default Realm:
    [RLMRealmConfiguration setDefaultConfiguration: config];
}

- (UserObject *) activeUser
{
    return [[UserObject allObjects] firstObject];
}

- (DeviceObject *) activeDevice
{
    DeviceObject *device = [DeviceObject objectForPrimaryKey: @(0)];

    if (!device) {
        device = [[DeviceObject alloc] init];
        device.id = 0;
        device.needsToBeCreated = YES;
    }

    return device;
}

- (CardObject *) activeUserCard
{
    UserObject *user = [self activeUser];
    if (!user) return nil;

    NSInteger cardId = user.cardId;
    if (cardId == 0) return nil;

    return [CardObject objectForPrimaryKey: @(cardId)];
}

@end
