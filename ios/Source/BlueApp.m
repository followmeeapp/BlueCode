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

#import "BlueClient.h"
#import "BlueModel.h"

#import "BlueDiscover.h"
#import "BlueDiscoverSession.h"

#import "UserObject.h"
#import "CardObject.h"

#import "BlueCardLinkController.h"

#import "ServerRequest.h"

typedef NS_ENUM(NSInteger, AppStatus) {
    Unknown = 0,
    ActiveJustLaunched,
    ActiveShowingBlueCardLink,
    ActiveShowingMain,
    InactiveShowingBlueCardLink,
    InactiveNoBluetooth,
    InactiveBluetoothNoBackground,
    InactiveBluetooth,
    InactiveAfterDeepLink,
    InactiveAfterFastFollowDeepLink,
};

NSString *uuidStr = @"47F97455-ECBA-47D3-A8E9-D8C69E537F46";

@interface BlueApp ()

@property (nonatomic, assign) AppStatus appStatus;

@property (nonatomic, strong) BlueDiscover *discover;

@end

@implementation BlueApp

- (instancetype) init
{
    if (self = [super init]) {
        _appStatus = ActiveJustLaunched;
    }

    return self;
}

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

    NSString *json = [NSString stringWithFormat: @"{\"d\":%@,\"c\":%@}", @(user.activeDeviceId), @(user.cardId)];
    NSLog(@"advertising username = %@", json);

    [self.discover startAdvertisingWithUsername: json];
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

        [sessions enumerateObjectsUsingBlock: ^(BlueDiscoverSession *session, NSUInteger idx, BOOL * _Nonnull stop) {
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

//                    NSPredicate *pred = [NSPredicate predicateWithFormat: @"id = %@", @(cardId)];
//                    CardObject *existingCard = [[CardObject objectsWithPredicate: pred] firstObject];
                    CardObject *existingCard = [CardObject objectForPrimaryKey: @(cardId)];

                    if (!existingCard) {
                        CardObject *newCard = [[CardObject alloc] init];

                        newCard.id = cardId;
                        newCard.version = 0;

                        [realm addOrUpdateObject: newCard];

                        NSData *cardRequestPacket = [ServerRequest newCardRequestWithId: [self.blueClient nextRequestId] cardId: cardId version: 0];
                        [self.blueClient sendRequest: cardRequestPacket];

                        NSLog(@"added card = %@", @(cardId));

                    } else {
                        NSLog(@"(INFO) already have card = %@", @(cardId));
                    }
                }
            }
        }];

        [realm commitWriteTransaction];
    }];
}

- (void) stopBluetoothDiscovery
{
    [self startBluetooth];

    if (!self.discover.isDiscovering) return;

    [self.discover stopDiscovering];
}

- (BOOL)
application:                   (UIApplication *) application
didFinishLaunchingWithOptions: (NSDictionary *)  launchOptions
{
    [self configureModel: application];
    [self configureClient: application];
    [self configureFabric: application];
    [self configureBranch: application withOptions: launchOptions];

//    [[UINavigationBar appearance] setBarTintColor: [self navBarColor]];
//    [[UINavigationBar appearance] setTranslucent: NO];

    [[UINavigationBar appearance] setBarTintColor: [self black]];
    [[UINavigationBar appearance] setTranslucent: YES];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    [application setStatusBarStyle: UIStatusBarStyleLightContent];

    // set back button color
    [[UIBarButtonItem appearanceWhenContainedIn: [UINavigationBar class], nil]
                      setTitleTextAttributes:    @{ UITextAttributeTextColor: [UIColor whiteColor] }
                      forState:                  UIControlStateNormal                              ];
#pragma clang diagnostic pop

    return YES;
}

- (void) applicationDidBecomeActive: (UIApplication *) application
{
    if (self.overlayWindow) {
        [self.overlayWindow resignKeyWindow];
        self.overlayWindow.hidden = YES;
    }

    [[NSNotificationCenter defaultCenter] postNotificationName: @"ContinueFastFollowIfNeeded" object: nil];
}

- (void) applicationWillResignActive: (UIApplication *) application
{
    if (!self.overlayWindow) {
        self.overlayWindow = [[UIWindow alloc] initWithFrame: [[UIScreen mainScreen] bounds]];
        self.overlayWindow.windowLevel = UIWindowLevelAlert + 1;

        UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
        UIViewController *vc = [sb instantiateViewControllerWithIdentifier: @"NoBluetooth"];

        self.overlayWindow.rootViewController = vc;
    }

    [self.overlayWindow makeKeyAndVisible];
}

- (void) showBluetoothMonitor
{
    if (!self.bluetoothMonitorWindow) {
        self.bluetoothMonitorWindow = [[UIWindow alloc] initWithFrame:CGRectMake(0, 80, 320, 320)];
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
    self.blueClient = [[BlueClient alloc] initWithSecretKey: nil publicKey: nil serverKey: nil hostname: @"199.19.87.92" port: 3000];
}

#pragma mark - Branch.io

- (void)
handleDeepLinkWithDictionary: (NSDictionary *) params
error:                        (NSError *)      error
{
    if (_appStatus == ActiveJustLaunched) {
        if ([[params objectForKey: @"+clicked_branch_link"] boolValue]) {
            // BlueCardLinkController manages its own view controllers, so we use a dummy one here.
            // TODO(Erich): Can we just pass nil to initWithRootViewController?
            UIViewController *vc = [[UIViewController alloc] init];
            BlueCardLinkController *nc = [[BlueCardLinkController alloc] initWithRootViewController: vc];

            _appStatus = ActiveShowingBlueCardLink;

            [self.window.rootViewController presentViewController: nc animated: YES completion: nil];

        } else {
            UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
            UIViewController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueMainNavigationController"];

            self.window.rootViewController = vc;
            _appStatus = ActiveShowingMain;

            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^(void) {
                [[NSNotificationCenter defaultCenter] postNotificationName: @"RefreshCards" object: nil];
            });
        }

    } else {
//        AlertWithMessage(@"Unhandled _appStatus");
    }
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
#pragma clang diagnostic pop

