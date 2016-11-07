//
//  BlueBackup.m
//  Follow
//
//  Created by Erich Ocean on 10/22/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueBackup.h"

#import <BZipCompression/BZipCompression.h>
#import <DigitsKit/DigitsKit.h>
#import <SVProgressHUD/SVProgressHUD.h>

#import "BlueApp.h"
#import "BlueClient.h"
#import "BlueModel.h"

#import "ServerRequest.h"

#import "UserObject.h"
#import "DeviceObject.h"
#import "CardObject.h"

#import "Utilities.h"

#include <iostream>
#include <string>

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/request.capnp.h"
#include "schema/card.capnp.h"
#include "schema/backup.capnp.h"

@interface BlueBackup ()

@property (nonatomic, strong) NSTimer *timer;

@property (nonatomic, assign) BOOL expectingConfirmation;
@property (nonatomic, assign) int64_t timestamp;

@property (atomic, assign) BOOL isHandlingBackup;
@property (atomic, assign) BOOL askedForABackup;
@property (atomic, assign) BOOL askedToLogOut;

@end

@implementation BlueBackup

- (instancetype) init
{
    if (self = [super init]) {
        self.timer = [NSTimer timerWithTimeInterval: 60.0 * 1.0 // Every minute
                              target:                self
                              selector:              @selector(backupIfNeeded:)
                              userInfo:              nil
                              repeats:               YES              ];

        // Make sure this is all done on the main thread.
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
            [[NSRunLoop currentRunLoop] addTimer: self.timer forMode: NSRunLoopCommonModes];

            [[NSNotificationCenter defaultCenter] addObserver: self
                                                  selector:    @selector(previousBackupNotApplied:)
                                                  name:        @"BluePreviousBackupNotApplied"
                                                  object:      nil                                ];

            [self backupIfNeeded: nil];
        });
    }

    return self;
}

- (void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];

    [self.timer invalidate];
    self.timer = nil;
}

- (void) previousBackupNotApplied: (NSNotification *) note
{
    // We need to load and apply the current backup.
    self.expectingConfirmation = NO;

    NSInteger requestId = [APP_DELEGATE.blueClient nextRequestId];
    [APP_DELEGATE.blueClient sendRequest: [ServerRequest newBackupRequestWithId: requestId]];
}

- (IBAction) backupNow: sender
{
    if (self.isHandlingBackup) return;

    // Don't check for a WebSocket, or delay how long we choose to backup, i.e. "force it".
    self.askedForABackup = YES;
    [self sendBackup];
}

- (IBAction) logout: sender
{
    if (self.isHandlingBackup) return;

    self.askedToLogOut = YES;
    [self backupNow: sender];
}

- (void) backupIfNeeded: (NSTimer *) timer
{
    if (self.isHandlingBackup) return;

    self.askedForABackup = NO;
    self.askedToLogOut = NO;

    // Only do backup stuff when we've got an open WebSocket connection.
    if (![APP_DELEGATE.blueClient hasWebSocket]) return;

    UserObject *user = [APP_DELEGATE.blueModel activeUser];
    if (!user) return;

    int64_t lastUpdate = user.lastBackup;
    int64_t now = [[NSDate dateWithTimeIntervalSinceNow: 0] millisecondsSince1970];

    if (lastUpdate == 0) {
        // We need to load a backup, if it exits.
        self.expectingConfirmation = NO;

        NSInteger requestId = [APP_DELEGATE.blueClient nextRequestId];
        [APP_DELEGATE.blueClient sendRequest: [ServerRequest newBackupRequestWithId: requestId]];

    } else if (lastUpdate > (now + 60000)) {
        // lastUpdate is bogus (more than 60 seconds in the future), fix it
        // Our backup succeeded. Update the User object accordingly.
        RLMRealm *realm = [RLMRealm defaultRealm];
        [realm beginWriteTransaction];

        user.lastBackup = 0;
        [realm addOrUpdateObject: user];

        [realm commitWriteTransaction];

        [self previousBackupNotApplied: nil]; // Request the latest backup so we can apply it.

    } else if (now - lastUpdate > 1000L * 60L * 60L * 24L) {
        [self sendBackup];
    }
}

- (void) sendBackup
{
    UserObject *user = [APP_DELEGATE.blueModel activeUser];
    if (!user) return;

    int64_t lastUpdate = user.lastBackup;
    int64_t now = [[NSDate dateWithTimeIntervalSinceNow: 0] millisecondsSince1970];

    self.expectingConfirmation = YES;
    self.timestamp = now;

    NSInteger requestId = [APP_DELEGATE.blueClient nextRequestId];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
        NSData *request = [ServerRequest newCreateBackupRequestWithId: requestId
                                         timestamp:                    now
                                         previous:                     lastUpdate];

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
            [APP_DELEGATE.blueClient sendRequest: request];
        });
    });
}

- (void)
handleBackupWithTimestamp: (int64_t)  timestamp
data:                      (NSData *) data
{
    self.isHandlingBackup = YES;

    UserObject *user = [APP_DELEGATE.blueModel activeUser];
    if (!user) return;

    int64_t lastUpdate = user.lastBackup;

    if (lastUpdate == 0 && timestamp == 0) {
        // There's no backup to apply, but we should try and save one.
        int64_t now = [[NSDate dateWithTimeIntervalSinceNow: 0] millisecondsSince1970];

        self.expectingConfirmation = YES;
        self.timestamp = now;

        NSInteger requestId = [APP_DELEGATE.blueClient nextRequestId];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
            NSData *request = [ServerRequest newCreateBackupRequestWithId: requestId
                                             timestamp:                    now
                                             previous:                     lastUpdate];

            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                [APP_DELEGATE.blueClient sendRequest: request];
            });
        });

        self.isHandlingBackup = NO;
        return;
    }

    if (lastUpdate >= timestamp) {
        // Strange, we got a backup that has already been applied?
        self.isHandlingBackup = NO;
        [self backupIfNeeded: nil];
        return;
    }

    int64_t now = [[NSDate dateWithTimeIntervalSinceNow: 0] millisecondsSince1970];
    if (timestamp > (now + 60000)) {
        // Something is wrong!
        NSLog(@"ERROR: Got a backup that is more than 60 seconds in the future? Not applying. %@", @(timestamp));
        self.isHandlingBackup = NO;
        return;
    }

    if (data == nil || data.length == 0) {
        if (self.expectingConfirmation && self.timestamp == timestamp) {
            self.expectingConfirmation = NO;

            // Our backup succeeded. Update the User object accordingly.
            RLMRealm *realm = [RLMRealm defaultRealm];
            [realm beginWriteTransaction];

            user = [APP_DELEGATE.blueModel activeUser];
            user.lastBackup = timestamp;
            [realm addOrUpdateObject: user];

            [realm commitWriteTransaction];

            if (self.askedForABackup) {
                self.askedForABackup = NO;

                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                    [SVProgressHUD setDefaultStyle: SVProgressHUDStyleCustom];
                    [SVProgressHUD setDefaultMaskType: SVProgressHUDMaskTypeBlack];
                    [SVProgressHUD setForegroundColor: [UIColor blackColor]];
                    [SVProgressHUD setBackgroundColor: [UIColor whiteColor]];

                    if (self.askedToLogOut) {
                        [SVProgressHUD showSuccessWithStatus: @"Logging out…"];

                    } else {
                        [SVProgressHUD showSuccessWithStatus: @"Saved!"];
                    }

                    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                        [SVProgressHUD dismiss];

                        if (self.askedToLogOut) {
                            [[Digits sharedInstance] logOut];

                            // Remove the Realm database and quit the app.
                            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                                [APP_DELEGATE.blueModel destroyRealmAndExit];
                            });
                        }
                    });
                });
            }
        }

        self.isHandlingBackup = NO;
        return;
    }

    // Apply the backup (on a background thread).
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
        NSError *error = nil;
        NSData *uncompressed = [BZipCompression decompressedDataWithData: data error: &error];
        if (error) {
            NSLog(@"Error decompressing data %@", error);
            self.isHandlingBackup = NO;
            return;
        }

        kj::ArrayPtr<const capnp::word> view((const capnp::word *)[uncompressed bytes], [uncompressed length] / 8);
        capnp::FlatArrayMessageReader reader(view);

        ClientBackup::Reader clientBackup = reader.getRoot<ClientBackup>();

        RLMRealm *realm = [RLMRealm defaultRealm];
        [realm beginWriteTransaction];

        auto kind = clientBackup.getKind();
        auto which = kind.which();

        bool didAddCard = false;

        if (which == ClientBackup::Kind::BACKUP_CARD_LIST) {
            UserObject *user = [APP_DELEGATE.blueModel activeUser];
            user.lastBackup = timestamp;
            [realm addOrUpdateObject: user];

            auto backupCardList = kind.getBackupCardList();
            NSLog(@"BACKUP CARD LIST: %@", @(backupCardList.size()));

            for (auto backupCard : backupCardList) {
                CardObject *existingCard = [CardObject objectForPrimaryKey: @(backupCard.getId())];

                if (existingCard == nil) {
                    NSLog(@"Adding card from backup with Card Id %@", @(backupCard.getId()));

                    CardObject *newCard = [[CardObject alloc] init];
                    newCard.id = backupCard.getId();
                    newCard.timestamp = [NSDate dateWithMillisecondsSince1970: backupCard.getTimestamp()];
                    newCard.isFromBackup = YES;
                    newCard.isBLECard = backupCard.getIsBLECard();
                    newCard.isFromBlueCardLink = backupCard.getIsBlueCardLink();

                    newCard.fullName = [NSString stringWithUTF8String: backupCard.getFullName().cStr()];

                    if (backupCard.hasLocation()) {
                        newCard.location = [NSString stringWithUTF8String: backupCard.getLocation().cStr()];
                    }

                    [realm addOrUpdateObject: newCard];
                    didAddCard = true;
                }
            }

            if (didAddCard) {
                NSLog(@"Updating DeviceObject with new cards.");

                // We need to update our active DeviceObject visible card list manually
                // and force a full refresh.
                DeviceObject *device = [APP_DELEGATE.blueModel activeDevice];
                CardObject *userCard = [APP_DELEGATE.blueModel activeUserCard];

                RLMResults<CardObject *> *allCards = [CardObject allObjects];
                RLMResults<CardObject *> *sortedCards = [allCards sortedResultsUsingProperty: @"timestamp" ascending: YES];

                NSInteger count = userCard ? sortedCards.count - 1 : sortedCards.count;
                if (count > 0) {
                    // Count should always be greater than zero, but be safe anyway...
                    NSMutableArray *visibleCardIds = [NSMutableArray arrayWithCapacity: count];
                    NSInteger idx = 0;
                    for (CardObject *card in sortedCards) {
                        if (card.id != userCard.id) {
                            visibleCardIds[idx++] = @(card.id);
                        }
                    }

                    [device updateVisibleCards: visibleCardIds hiddenCards: @[]];
                    [realm addOrUpdateObject: device];

                } else if (device.needsToBeCreated) {
                    [realm addOrUpdateObject: device];
                }
            }

        } else {
            NSLog(@"Unknown backup kind: %@", @(which));
        }

        [realm commitWriteTransaction];

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
            if (didAddCard) {
                [[NSNotificationCenter defaultCenter] postNotificationName: @"BlueDidUpdateCardsFromBackup" object: nil];
            }

            self.isHandlingBackup = NO;
            [self backupIfNeeded: nil];
        });
    });
}

@end
