//
//  root.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Shared.h"
#import "Follow-private.h"
#define STATE_NAME ROOT

#import <Branch/Branch.h>

#import <FBSDKCoreKit/FBSDKCoreKit.h>

#import <Fabric/Fabric.h>
#import <Crashlytics/Crashlytics.h>
#import <DigitsKit/DigitsKit.h>
#import <TwitterKit/TwitterKit.h>

#import "IXKeychain.h"

#import "RootState.h"

static const int ddLogLevel __attribute__((unused)) = LOG_LEVEL_DEBUG;

@interface STATE : RootState
@end

@implementation STATE

SUBSTATES( OFFLINE, 0, 3, 4, 5, 6, 7, 8 )

#pragma - UIApplicationDelegate

- (void) applicationDidBecomeActive: (UIApplication *) application
{
    [self handled];

    [FBSDKAppEvents activateApp];
}

- (BOOL)
application:                   (UIApplication *) application
didFinishLaunchingWithOptions: (NSDictionary *)  launchOptions
{
    [self handled];

    [APP_CONTROLLER startReachability];
    [FOLLOW_CONTROLLER createUserInterface];

    [self gotoState: [self substateWithPath: @"OFFLINE.CONNECTING"]];

    [[Branch getInstance] initSessionWithLaunchOptions: launchOptions
                          andRegisterDeepLinkHandler:
    ^(NSDictionary *params, NSError *error) {
        // params are the deep linked params associated with the link that the user clicked before showing up.
        NSLog(@"deep link data: %@", [params description]);

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^(void) {
            [APP_CONTROLLER handleDeepLinkWithDictionary: params error: error];
        });
    }];

    [Fabric with: @[[Twitter class], [Digits class], [Crashlytics class]]];

    [[FBSDKApplicationDelegate sharedInstance] application:                   application
                                               didFinishLaunchingWithOptions: launchOptions];

    return YES;
}

- (BOOL)
application:       (UIApplication *) application
openURL:           (NSURL *)         url
sourceApplication: (NSString *)      sourceApplication
annotation:        (id)              annotation
{
    // FIXME: Do anything with the result?
    BOOL handled = [[FBSDKApplicationDelegate sharedInstance] application:       application
                                                              openURL:           url
                                                              sourceApplication: sourceApplication
                                                              annotation:        annotation       ];

    [[Branch getInstance] handleDeepLink: url];

    (void)handled; // suppress compiler warning

    return YES;
}

- (BOOL)
application:          (UIApplication *)     application
continueUserActivity: (NSUserActivity *)    userActivity
restorationHandler:   (void (^)(NSArray *)) restorationHandler
{
    [self handled];

    [[Branch getInstance] continueUserActivity: userActivity];

    // TODO: Do I need to call restorationHandler here?

    return YES;
}

- (NSUInteger)
application:                             (UIApplication *) application
supportedInterfaceOrientationsForWindow: (UIWindow *)      window
{
    [self handled];

    return UIInterfaceOrientationMaskPortrait;
}

//- (void)
//handleDeepLinkWithDictionary: (NSDictionary *) params
//error:                        (NSError *)      error
//{
//    if (error) {
//        [APP_CONTROLLER sendAction: @"handleDeepLinkError" withArguments: @{ @"error": [error description] }];
//
//    } else {
//        [APP_CONTROLLER sendAction: @"handleDeepLink" withArguments: params];
//    }
//}

- (void) webSocketStatusDidChange: (BOOL) hasWebSocket
{
    if (hasWebSocket) {
        [self handled];
        
    } else {
        [self gotoState: [self substateWithPath: @"OFFLINE.CONNECTION_FAILED"]];
    }
}

- (void) internetStatusDidChange: (BOOL) hasInternet
{
    if (!hasInternet) {
        [self gotoState: [self substateWithPath: @"OFFLINE.SERVER_UNREACHABLE"]];

    } else {
        [self handled];
    }
}

@end
