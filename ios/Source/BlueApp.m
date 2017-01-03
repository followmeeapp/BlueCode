//
//  BlueApp.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueApp.h"

#import <Branch/Branch.h>

#import <Fabric/Fabric.h>
#import <Crashlytics/Crashlytics.h>
#import <DigitsKit/DigitsKit.h>

@import Firebase;
@import GoogleMobileAds;

#import "BlueClient.h"
#import "BlueModel.h"
#import "BlueBackup.h"

#import "BlueDiscover.h"
#import "BlueDiscoverSession.h"

#import "UserObject.h"
#import "DeviceObject.h"
#import "CardObject.h"

#import "BlueCardLinkController.h"

#import "ServerRequest.h"

#import "BlueIntroController.h"
#import "BlueMainController.h"
#import "BlueRegisterController.h"

#import "Utilities.h"

NSString *uuidStr = @"E5586A8D-F8A9-4557-B123-6CB070B0AA71";

@interface BlueApp () <UIAlertViewDelegate>

@property (nonatomic, strong) BlueDiscover *discover;

@property (nonatomic, assign) BOOL upgradeRequired;

@property (nonatomic, assign) BOOL alreadySetUp;

@end

@implementation BlueApp

- (void) startBluetooth
{
    if (self.discover) return;

    CBUUID *uuid = [CBUUID UUIDWithString: uuidStr];
    self.discover = [[BlueDiscover alloc] initWithUUID: uuid];
}

- (void) startBluetoothAdvertising
{
    [self startBluetooth];

    if (self.discover.isAdvertising) return;

    UserObject *user = [self.blueModel activeUser];
    CardObject *card = [self.blueModel activeUserCard];

    if (!user || !card) {
        NSLog(@"advertising failed: no user or no card");
        return;
    }

    [self.discover startAdvertisingWithDeviceId: user.activeDeviceId card: card];
}

- (void) stopBluetoothAdvertising
{
    [self startBluetooth];

    if (!self.discover.isAdvertising) return;

    [self.discover stopAdvertising];
}

- (void) startBluetoothDiscovery
{
    [self startBluetooth];

    if (self.discover.isDiscovering) return;

    [self.discover startDiscovering: ^(NSArray *sessions, BOOL sessionsChanged) {
        if (!sessionsChanged) return;
        if (sessions.count == 0) return;

        RLMRealm *realm = [RLMRealm defaultRealm];

        [realm beginWriteTransaction];

        __block BOOL didUpdateCards = NO;

        DeviceObject *device = [self.blueModel activeDevice];

        NSMutableArray *visibleCards = [[[device.visibleCards reverseObjectEnumerator] allObjects] mutableCopy];
        NSMutableArray *hiddenCards = [[[device.hiddenCards reverseObjectEnumerator] allObjects] mutableCopy];

        [sessions enumerateObjectsUsingBlock: ^(BlueDiscoverSession *session, NSUInteger idx, BOOL *stop) {
            if (!session.isFinished) return;

            NSString *json = session.username;
            if (json) {
                NSLog(@"json = %@", json);

                NSError *error = nil;
                NSDictionary *dict = [NSJSONSerialization JSONObjectWithData: [json dataUsingEncoding: NSUTF8StringEncoding]
                                                          options:            0
                                                          error:              &error                                       ];

                if (error) {
                    NSLog(@"JSON parse error: %@", error);

                } else {
                    NSInteger cardId = [dict[@"c"] integerValue];
                    if (cardId == 0) return;

                    CardObject *existingCard = [CardObject objectForPrimaryKey: @(cardId)];

                    if (!existingCard) {
                        CardObject *newCard = [[CardObject alloc] init];

                        [APP_DELEGATE.blueAnalytics discoverCardViaBluetooth: cardId];

                        newCard.id = cardId;
                        newCard.isBLECard = YES;
                        newCard.version = 0;

                        newCard.fullName = session.fullName;

                        NSString *location = session.location;
                        if (location.length > 0) newCard.location = location;

                        [realm addOrUpdateObject: newCard];

                        // By definition, the card cannot already be in visibleCards.
                        [visibleCards addObject: @(newCard.id)];
                        didUpdateCards = YES;

                        NSData *cardRequestPacket = [ServerRequest newCardRequestWithId: [self.blueClient nextRequestId] cardId: cardId version: 0];
                        [self.blueClient sendRequest: cardRequestPacket];

                        NSLog(@"added card = %@", @(cardId));

                    } else {
                        NSLog(@"(INFO) already have card = %@", @(cardId));
                        int cardVersion = [dict[@"v"] intValue];

                        // [1] Workaround a bug where we incorrectly set version to -1 (which is invalid).
                        if (existingCard.version == -1 || existingCard.version < cardVersion) {
                            // The card was updated (and we just learned this via Bluetooth LE), so we need to refresh it.
                            int oldVersion = existingCard.version;

                            // [2] Continue workaround...
                            if (oldVersion == -1) {
                                oldVersion = 0;
                                existingCard.version = 0;
                            }

                            existingCard.fullName = session.fullName;

                            NSString *location = session.location;
                            existingCard.location = (location.length > 0) ? location : nil;

                            [realm addOrUpdateObject: existingCard];

                            if (existingCard.visibleSection != 0) {
                                // We may need to add this to our hiddenSection.
                                if ([hiddenCards indexOfObject: @(existingCard.id)] != NSNotFound) {
                                    // Only update the timestamp when we added it to recents the very first time?
                                    existingCard.timestamp = [NSDate dateWithTimeIntervalSinceNow: 0];

                                    // Note: We don't record that it's in recents in the existingCard.sections object.
                                    [hiddenCards addObject: @(existingCard.id)];
                                    didUpdateCards = YES;
                                }
                            }

                            NSData *cardRequestPacket = [ServerRequest newCardRequestWithId: [self.blueClient nextRequestId] cardId: cardId version: oldVersion];
                            [self.blueClient sendRequest: cardRequestPacket];

                            NSLog(@"refreshing card = %@, from version = %@ to version = %@", @(cardId), @(oldVersion), @(cardVersion));
                        }
                    }
                }
            }
        }];

        if (didUpdateCards) {
            [device updateVisibleCards: visibleCards hiddenCards: hiddenCards];
        }

        if (didUpdateCards || device.needsToBeCreated) [realm addOrUpdateObject: device];

        [realm commitWriteTransaction];
    }];
}

- (void) stopBluetoothDiscovery
{
    [self startBluetooth];

    if (!self.discover.isDiscovering) return;

    [self.discover stopDiscovering];
}

- (void) startBlueBackup
{
  if (self.blueBackup) return;

  self.blueBackup = [[BlueBackup alloc] init];
}

- (BOOL)
application:                   (UIApplication *) application
didFinishLaunchingWithOptions: (NSDictionary *)  launchOptions
{
    [self configureModel: application]; // Must come first, since it configures the default Realm database used elsewhere.

    [self configureClient: application];
    [self configureFabric: application];
    [self configureBranch: application withOptions: launchOptions];
    [self configureAdMob: application];
    [self configureAnalytics: application];

    [[UINavigationBar appearance] setBarTintColor: [self grey]];
    [[UINavigationBar appearance] setTranslucent: NO];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    [application setStatusBarStyle: UIStatusBarStyleDefault];

    // set back button color
    UIColor *tintColor = APP_DELEGATE.black;
    [[UIBarButtonItem appearanceWhenContainedIn: [UINavigationBar class], nil]
                      setTitleTextAttributes:    @{ UITextAttributeTextColor: tintColor }
                      forState:                  UIControlStateNormal                   ];
#pragma clang diagnostic pop

    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(upgradeRequired:) name: @"UpgradeRequired" object: nil];
    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(joinRequired:) name: @"JoinRequired" object: nil];

    [self startBluetoothDiscovery];

    UserObject *user = [APP_DELEGATE.blueModel activeUser];
    if (user) {
        [self startBlueBackup];
    }

    if (user && user.cardId > 0) {
        [self startBluetoothAdvertising];
    }

    return YES;
}

- (void) upgradeRequired: (NSNotification *) note
{
    self.upgradeRequired = YES;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Upgrade Required"
                                                  message:           @"Your app is out-of-date. Please upgrade to the latest version to continue."
                                                  delegate:          self
                                                  cancelButtonTitle: @"OK"
                                                  otherButtonTitles: nil                                                                         ];
    [alertView show];
#pragma clang diagnostic pop
}

- (void) joinRequired: (NSNotification *) note
{
    // We only need to force a join if we've got an active user.
    if (![self.blueModel activeUser]) return;

    DGTAppearance *appearance = [[DGTAppearance alloc] init];
    appearance.backgroundColor = APP_DELEGATE.blue1;
    appearance.accentColor = [UIColor whiteColor];

    DGTAuthenticationConfiguration *configuration = [[DGTAuthenticationConfiguration alloc] initWithAccountFields: DGTAccountFieldsEmail];
    configuration.appearance = appearance;
    configuration.title = @"Join Blue";

    [[Digits sharedInstance] authenticateWithViewController: nil configuration: configuration completion: ^(DGTSession *session, NSError *error) {
        if (error) {
            [self.blueAnalytics digitsLoginFail];

            // FIXME(Erich): HACK, causes the app to exit(0).
            self.upgradeRequired = YES;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Error re-registering with Blue"
                                                          message:           @"Please try again later."
                                                          delegate:          self
                                                          cancelButtonTitle: @"OK"
                                                          otherButtonTitles: nil                             ];
            [alertView show];
#pragma clang diagnostic pop

        } else {
            [self.blueAnalytics digitsLoginSuccess];

            UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
            BlueRegisterController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueRegisterController"];
            UINavigationController *nc = [[UINavigationController alloc] initWithRootViewController: vc];

            vc.shouldDismissNavigationController = YES;

            [self.window.rootViewController presentViewController: nc animated: YES completion: ^{
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.001 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                    [vc digitsAuthenticationFinishedWithSession: session error: error];
                });
            }];
        }
    }];
}

#pragma mark UIAlertViewDelegate

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
- (void)
alertView:            (UIAlertView *) alertView
clickedButtonAtIndex: (NSInteger)     buttonIndex
{
    if (self.upgradeRequired) {
        exit(0);
    }
}
#pragma clang diagnostic pop

- (void) applicationDidBecomeActive: (UIApplication *) application
{
//    if (self.overlayWindow) {
//        [self.overlayWindow resignKeyWindow];
//        self.overlayWindow.hidden = YES;
//    }
}

- (void) applicationWillResignActive: (UIApplication *) application
{
//    if (!self.overlayWindow) {
//        self.overlayWindow = [[UIWindow alloc] initWithFrame: [[UIScreen mainScreen] bounds]];
//        self.overlayWindow.windowLevel = UIWindowLevelAlert + 1;
//
//        UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
//        UIViewController *vc = [sb instantiateViewControllerWithIdentifier: @"NoBluetooth"];
//
//        self.overlayWindow.rootViewController = vc;
//    }
//
//    [self.overlayWindow makeKeyAndVisible];
}

- (void) showBluetoothMonitor
{
    if (!self.bluetoothMonitorWindow) {
        self.bluetoothMonitorWindow = [[UIWindow alloc] initWithFrame: [[UIScreen mainScreen] bounds]];
        self.bluetoothMonitorWindow.windowLevel = UIWindowLevelAlert;

        UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
        UIViewController *vc = [sb instantiateViewControllerWithIdentifier: @"BluetoothMonitor"];

        self.bluetoothMonitorWindow.rootViewController = vc;
    }

    [self.bluetoothMonitorWindow makeKeyAndVisible];
}

- (void) hideBluetoothMonitor
{
    if (self.bluetoothMonitorWindow) {
        [self.bluetoothMonitorWindow resignKeyWindow];
        self.bluetoothMonitorWindow.hidden = YES;
    }
}

- (BOOL)
application:       (UIApplication *) application
openURL:           (NSURL *)         url
sourceApplication: (NSString *)      sourceApplication
annotation:        (id)              annotation
{
    [[Branch getInstance] handleDeepLink: url];

    return YES;
}

- (BOOL)
application:          (UIApplication *)     application
continueUserActivity: (NSUserActivity *)    userActivity
restorationHandler:   (void (^)(NSArray *)) restorationHandler
{
    [[Branch getInstance] continueUserActivity: userActivity];

    // TODO: Do I need to call restorationHandler here?

    return YES;
}

- (void)
configureBranch: (UIApplication *) application
withOptions:     (NSDictionary *)  launchOptions
{
    [[Branch getInstance] initSessionWithLaunchOptions: launchOptions
                          andRegisterDeepLinkHandler:
    ^(NSDictionary *params, NSError *error) {
        // params are the deep linked params associated with the link that the user clicked before showing up.
        NSLog(@"deep link data: %@", [params description]);

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^(void) {
            [self handleDeepLinkWithDictionary: params error: error];
        });
    }];
}

- (void) configureFabric: (UIApplication *) application
{
    [Fabric with: @[[Digits class], [Crashlytics class]]];
}

- (void) configureModel: (UIApplication *) application
{
    self.blueModel = [[BlueModel alloc] init];
}

- (void) configureClient: (UIApplication *) application
{
//#ifdef DEBUG
//    NSData *serverKey = [NSData dataFromHexString: @"bf2a07a4be1ed94767e2e0977369a216d59823e072dd4d67a927fe9da3d9b361"];
//    self.blueClient = [[BlueClient alloc] initWithServerKey: serverKey hostname: @"192.168.0.109" port: 3000];
//#else
    NSData *serverKey = [NSData dataFromHexString: @"3be351d8f39c2216e466564a452fe106dd42487528d3c0ba5b0ed9691106336d"];
    self.blueClient = [[BlueClient alloc] initWithServerKey: serverKey hostname: @"199.19.87.92" port: 3000];
//#endif
}

- (void) configureAdMob: (UIApplication *) application
{
    [FIRApp configure];
    [GADMobileAds configureWithApplicationID: @"ca-app-pub-9762460744660147~5971587314"];
}

- (void) configureAnalytics: (UIApplication *) application
{
    self.blueAnalytics = [[BlueAnalytics alloc] init];
}

#pragma mark - Screen Capture

// From http://stackoverflow.com/a/3635080
- (UIImage *) captureScreen
{
  UIWindow *keyWindow = [[UIApplication sharedApplication] keyWindow];
  CGRect rect = [keyWindow bounds];
  UIGraphicsBeginImageContext(rect.size);
  CGContextRef context = UIGraphicsGetCurrentContext();
  [keyWindow.layer renderInContext:context];
  UIImage *img = UIGraphicsGetImageFromCurrentImageContext();
  UIGraphicsEndImageContext();
  return img;
}

#pragma mark - Branch.io

- (void)
handleDeepLinkWithDictionary: (NSDictionary *) params
error:                        (NSError *)      error
{
    if (!self.alreadySetUp) {
        self.alreadySetUp = YES;

        if (self.blueModel.activeUserCard == nil) {
            self.didShowIntro = YES;

            BlueIntroController *vc = [[BlueIntroController alloc] init];

            if ([[params objectForKey: @"+clicked_branch_link"] boolValue]) {
                vc.delayShowingIntro = YES;
            }

            self.window.rootViewController = vc;

        } else {
            [self showMain];
        }
    }

    if ([[params objectForKey: @"+clicked_branch_link"] boolValue]) {
        // BlueCardLinkController manages its own view controllers, so we use a dummy one here.
        UIViewController *vc = [[UIViewController alloc] init];
        BlueCardLinkController *nc = [[BlueCardLinkController alloc] initWithRootViewController: vc];

        nc.cardId = [params[@"cardId"] integerValue];
        nc.fullName = params[@"fullName"];

        NSString *location = params[@"location"];
        nc.location = (!location || (NSNull *)location == [NSNull null]) ? nil : location;

        [self.blueAnalytics viewCardViaCardLink: nc.cardId];

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^(void) {
            [nc configure];

            [self.window.rootViewController presentViewController: nc animated: YES completion: nil];
        });
    }
}

- (void) showMainAndCreateCard
{
    [self showMain: YES];
}

- (void) showMain
{
    [self showMain: NO];
}

- (void) showMain: (BOOL) shouldCreateCard
{
    UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
    UINavigationController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueMainNavigationController"];

    self.window.rootViewController = vc;

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^(void) {
        if (shouldCreateCard) {
            UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
            BlueRegisterController *completionViewController = [sb instantiateViewControllerWithIdentifier: @"BlueRegisterController"];

            DGTAppearance *appearance = [[DGTAppearance alloc] init];
            appearance.backgroundColor = APP_DELEGATE.blue1;
            appearance.accentColor = [UIColor whiteColor];

            DGTAuthenticationConfiguration *configuration = [[DGTAuthenticationConfiguration alloc] initWithAccountFields: DGTAccountFieldsEmail];
            configuration.appearance = appearance;
            configuration.title = @"Join Blue";

            [[Digits sharedInstance] authenticateWithNavigationViewController: self.window.rootViewController.navigationController
                                     configuration:                            configuration
                                     completionViewController:                 completionViewController                           ];

        } else {
            [[NSNotificationCenter defaultCenter] postNotificationName: @"RefreshCards" object: nil];
        }
    });
}

- (void) prepareToEditUserCard
{
    CardObject *card = [self.blueModel activeUserCard];

    self.didUpdateAvatar = NO;
    self.didUpdateCoverPhoto = NO;

    if (!card) {
        self.isUpdatingCard = NO;

        NSDictionary *properties = [NSDictionary dictionaryWithContentsOfURL: [self localPropertiesURL]];
        if (properties) {
            self.fullName = properties[@"fullName"];
            self.location = properties[@"location"];
            self.bio = properties[@"bio"];

            self.backgroundOpacity = [properties[@"backgroundOpacity"] doubleValue];
            self.rowOffset = [properties[@"rowOffset"] integerValue];

            NSMutableDictionary *networks = [NSMutableDictionary dictionary];
            [(NSArray *)properties[@"networks"] enumerateObjectsUsingBlock: ^(NSDictionary *dict, NSUInteger idx, BOOL *stop) {
                networks[dict[@"networkType"]] = dict[@"username"];
            }];
            self.networks = networks;

            self.originalAvatar     = [UIImage imageWithData: [NSData dataWithContentsOfURL: [self localOriginalAvatarURL]]];
            self.originalCoverPhoto = [UIImage imageWithData: [NSData dataWithContentsOfURL: [self localOriginalCoverPhotoURL]]];
            self.croppedAvatar      = [UIImage imageWithData: [NSData dataWithContentsOfURL: [self localCroppedAvatarURL]]];
            self.croppedCoverPhoto  = [UIImage imageWithData: [NSData dataWithContentsOfURL: [self localCroppedCoverPhotoURL]]];

        } else {
            // Otherwise, reset the editable fields.
            self.fullName = nil;
            self.location = nil;
            self.bio = nil;

            self.backgroundOpacity = 0.5;
            self.rowOffset = 0;

            self.networks = [NSMutableDictionary dictionary];

            self.originalAvatar = nil;
            self.originalCoverPhoto = nil;
            self.croppedAvatar = nil;
            self.croppedCoverPhoto = nil;
        }

    } else {
        self.isUpdatingCard = YES;
        self.card = card;

        self.fullName = card.fullName;
        self.location = card.location;
        self.bio = card.bio;

        self.backgroundOpacity = card.backgroundOpacity;
        self.rowOffset = card.rowOffset;

        NSMutableDictionary *networks = [NSMutableDictionary dictionary];
        for (NSInteger idx=0, len=card.networks.count; idx<len; ++idx) {
            NetworkObject *network = card.networks[idx];
            networks[@(network.type)] = network.username;
        }

        self.networks = networks;

        self.originalAvatar     = [UIImage imageWithData: [NSData dataWithContentsOfURL: [self localOriginalAvatarURL]]];
        self.originalCoverPhoto = [UIImage imageWithData: [NSData dataWithContentsOfURL: [self localOriginalCoverPhotoURL]]];
        self.croppedAvatar      = [UIImage imageWithData: [NSData dataWithContentsOfURL: [self localCroppedAvatarURL]]];
        self.croppedCoverPhoto  = [UIImage imageWithData: [NSData dataWithContentsOfURL: [self localCroppedCoverPhotoURL]]];
    }
}

- (void) saveTemporaryCardProperties
{
    NSMutableDictionary *props = [NSMutableDictionary dictionary];

    if (self.fullName) props[@"fullName"] = self.fullName;
    if (self.location) props[@"location"] = self.location;
    if (self.bio) props[@"bio"] = self.bio;

    props[@"backgroundOpacity"] = @(self.backgroundOpacity);
    props[@"rowOffset"] = @(self.rowOffset);

    NSMutableArray *networks = [NSMutableArray array];
    [self.networks enumerateKeysAndObjectsUsingBlock: ^(NSNumber *networkType, NSString *username, BOOL *stop) {
        [networks addObject: @{
            @"networkType": networkType,
            @"username": username,
        }];
    }];
    props[@"networks"] = networks;

    NSError *error = nil;
    NSData *properties = [NSPropertyListSerialization dataWithPropertyList: props format: NSPropertyListXMLFormat_v1_0 options: 0 error: &error];
    if (!properties) {
        NSLog(@"Plist serialization error: %@", error);
        // Don't make any changes.
        return;
    }

    [properties writeToURL: [self localPropertiesURL] atomically: YES];

    NSData *originalAvatar     = self.originalAvatar     ? UIImageJPEGRepresentation(self.originalAvatar, 1.0) : nil;
    NSData *originalCoverPhoto = self.originalCoverPhoto ? UIImageJPEGRepresentation(self.originalCoverPhoto, 1.0) : nil;
    NSData *croppedAvatar      = self.croppedAvatar      ? UIImageJPEGRepresentation(self.croppedAvatar, 1.0) : nil;
    NSData *croppedCoverPhoto  = self.croppedCoverPhoto  ? UIImageJPEGRepresentation(self.croppedCoverPhoto, 1.0): nil;

    // Save or remove the image data, as needed.
    if (originalAvatar) {
        [originalAvatar writeToURL: [self localOriginalAvatarURL] atomically: YES];

    } else {
        [[NSFileManager defaultManager] removeItemAtURL: [self localOriginalAvatarURL] error: nil];
    }

    if (originalCoverPhoto) {
        [originalCoverPhoto writeToURL: [self localOriginalCoverPhotoURL] atomically: YES];

    } else {
        [[NSFileManager defaultManager] removeItemAtURL: [self localOriginalCoverPhotoURL] error: nil];
    }

    if (croppedAvatar) {
        [croppedAvatar writeToURL: [self localCroppedAvatarURL] atomically: YES];

    } else {
        [[NSFileManager defaultManager] removeItemAtURL: [self localCroppedAvatarURL] error: nil];
    }

    if (croppedCoverPhoto) {
        [croppedCoverPhoto writeToURL: [self localCroppedCoverPhotoURL] atomically: YES];

    } else {
        [[NSFileManager defaultManager] removeItemAtURL: [self localCroppedCoverPhotoURL] error: nil];
    }
}

- (NSURL *) localPropertiesURL
{
    return [[self applicationDocumentsDirectory] URLByAppendingPathComponent: @"card.plist"];
}

- (NSURL *) localOriginalAvatarURL
{
    return [[self applicationDocumentsDirectory] URLByAppendingPathComponent: @"original_avatar.jpg"];
}

- (NSURL *) localOriginalCoverPhotoURL
{
    return [[self applicationDocumentsDirectory] URLByAppendingPathComponent: @"original_cover_photo.jpg"];
}

- (NSURL *) localCroppedAvatarURL
{
    return [[self applicationDocumentsDirectory] URLByAppendingPathComponent: @"cropped_avatar.jpg"];
}

- (NSURL *) localCroppedCoverPhotoURL
{
    return [[self applicationDocumentsDirectory] URLByAppendingPathComponent: @"cropped_cover_photo.jpg"];
}

- (NSURL *) localAvatarURL
{
    return [[self applicationDocumentsDirectory] URLByAppendingPathComponent: @"avatar.jpg"];
}

- (NSURL *) localCoverPhotoURL
{
    return [[self applicationDocumentsDirectory] URLByAppendingPathComponent: @"cover_photo.jpg"];
}

// See https://developer.apple.com/library/ios/technotes/tn2406/_index.html
- (NSURL *) applicationDocumentsDirectory
{
    return [[[NSFileManager defaultManager] URLsForDirectory: NSDocumentDirectory inDomains: NSUserDomainMask] lastObject];
}

@end

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
UIAlertView *AlertWithMessage(NSString *message)
{
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Unexpected Error"
                                                  message:           message
                                                  delegate:          nil
                                                  cancelButtonTitle: @"OK"
                                                  otherButtonTitles: nil               ];
    [alertView show];
    return alertView;
}
UIAlertView *ValidationAlertWithMessage(NSString *message)
{
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Validation Error"
                                                  message:           message
                                                  delegate:          nil
                                                  cancelButtonTitle: @"OK"
                                                  otherButtonTitles: nil               ];
    [alertView show];
    return alertView;
}
#pragma clang diagnostic pop

