//
//  Follow.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Follow.h"

#import <LayerKit/LayerKit.h>

#import <Branch/Branch.h>

#import <FBSDKCoreKit/FBSDKCoreKit.h>

#import <Fabric/Fabric.h>
#import <Crashlytics/Crashlytics.h>
#import <DigitsKit/DigitsKit.h>
#import <TwitterKit/TwitterKit.h>

#import "ConversationViewController.h"

#import "CardDrop.h"

// Layer App ID from developer.layer.com
static NSString *const LQSLayerAppIDString = @"layer:///apps/staging/4eb65614-3681-11e6-bcbc-f60d000000b0";

NSString *const ConversationMetadataDidChangeNotification = @"LSConversationMetadataDidChangeNotification";
NSString *const ConversationParticipantsDidChangeNotification = @"LSConversationParticipantsDidChangeNotification";
NSString *const ConversationDeletedNotification = @"LSConversationDeletedNotification";

#if TARGET_IPHONE_SIMULATOR
    // If on simulator set the user ID to Simulator and participant to Device
    NSString *const FollowCurrentUserID = @"Simulator";
    NSString *const FollowParticipantUserID = @"Device";
    NSString *const FollowInitialMessageText = @"Hey Device! This is your friend, Simulator.";
#else
    // If on device set the user ID to Device and participant to Simulator
    NSString *const FollowCurrentUserID = @"Device";
    NSString *const FollowParticipantUserID = @"Simulator";
    NSString *const FollowInitialMessageText =  @"Hey Simulator! This is your friend, Device.";
#endif

NSString *const FollowParticipant2UserID = @"Dashboard";
NSString *const FollowCategoryIdentifier = @"category_lqs";
NSString *const FollowAcceptIdentifier = @"ACCEPT_IDENTIFIER";
NSString *const FollowIgnoreIdentifier = @"IGNORE_IDENTIFIER";

@implementation Follow

- (void) applicationDidBecomeActive: (UIApplication *) application
{
    [FBSDKAppEvents activateApp];
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
    [[Branch getInstance] continueUserActivity: userActivity];

    // TODO: Do I need to call restorationHandler here?

    return YES;
}

- (BOOL)
application:                   (UIApplication *) application
didFinishLaunchingWithOptions: (NSDictionary *)  launchOptions
{
    [self configureRealm: application];
    [self configureLayer: application];
    [self configureServer: application];
    [self configureFabric: application];
    [self configureBranch: application withOptions: launchOptions];
    [self configureFacebook: application withOptions: launchOptions];

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
    [Fabric with: @[[Twitter class], [Digits class], [Crashlytics class]]];
}

- (void) configureServer: (UIApplication *) application
{
    self.serverClient = [[ServerClient alloc] initWithSecretKey: nil publicKey: nil serverKey: nil hostname: @"192.168.0.132" port: 3000];
}

- (void)
configureFacebook: (UIApplication *) application
withOptions:       (NSDictionary *)  launchOptions
{
    [[FBSDKApplicationDelegate sharedInstance] application:                   application
                                               didFinishLaunchingWithOptions: launchOptions];
}

- (void) configureRealm: (UIApplication *) application
{
    // Get the default Realm. You only need to do this once (per thread).
    RLMRealm *realm = [RLMRealm defaultRealm];

    // Destroy existing objects.
    [realm beginWriteTransaction];
    [realm deleteAllObjects];
    [realm commitWriteTransaction];

    // Create two CardDrop objects.
    CardDrop *drop = [[CardDrop alloc] init];
    drop.id = 1;
    drop.displayName = @"Steven Spielberg";
    drop.latitude = 34.0522;
    drop.longitude = -118.2437;

    CardDrop *drop2 = [[CardDrop alloc] init];
    drop2.id = 2;
    drop2.displayName = @"Woody Allen";
    drop2.latitude = 40.7128;
    drop2.longitude = -74.0059;

    // Add CardDrop objects to Realm with transaction.
    [realm beginWriteTransaction];
    [realm addObject: drop];
    [realm addObject: drop2];
    [realm commitWriteTransaction];
}

- (void) configureLayer: (UIApplication *) application
{
    if (self.layerClient.isConnected) return; // We're already set up and ready to go.

    if (!self.layerClient) {
        // Add support for shake gesture
        application.applicationSupportsShakeToEdit = YES;

        // Initializes a LYRClient object
        NSURL *appID = [NSURL URLWithString: LQSLayerAppIDString];
        self.layerClient = [LayerClient clientWithAppID: appID];
        self.layerClient.delegate = self;
        self.layerClient.autodownloadMIMETypes = [NSSet setWithObjects: @"image/png", nil];
        self.layerClient.backgroundContentTransferEnabled = YES;

        // Register for push
        [self registerApplicationForPushNotifications: application];
    }

    // Connect to Layer. See "Quick Start - Connect" for more details:
    // https://developer.layer.com/docs/quick-start/ios#connect
    [self.layerClient connectWithCompletion: ^(BOOL success, NSError *error) {
        if (!success) {
            NSLog(@"Failed to connect to Layer: %@", error);

        } else {
            [self authenticateLayerWithUserID: FollowCurrentUserID completion: ^(BOOL success, NSError *error) {
                if (!success) {
                    NSLog(@"Failed Authenticating Layer Client with error:%@", error);
                }
            }];
        }
    }];
}

#pragma mark - Branch.io

- (void)
handleDeepLinkWithDictionary: (NSDictionary *) params
error:                        (NSError *)      error
{
//    if (error) {
//        self.deepLinkAction = @"handleDeepLinkError";
//        self.deepLinkData = @{ @"error": [error description] };
//        
//    } else {
//        self.deepLinkAction = @"handleDeepLink";
//        self.deepLinkData = params;
//    }
//
//    if (!_client) return;
//
//    [self sendAction: self.deepLinkAction withArguments: self.deepLinkData];
}

#pragma mark - Push Notification Methods

- (void) registerApplicationForPushNotifications: (UIApplication *) application
{
    // Set up push notifications. For more information about Push, check out:
    // https://developer.layer.com/docs/ios/guides#push-notification

    UIMutableUserNotificationAction *acceptAction = [[UIMutableUserNotificationAction alloc] init];
    acceptAction.identifier = FollowAcceptIdentifier;
    acceptAction.title = @"Show me!";
    acceptAction.activationMode = UIUserNotificationActivationModeBackground;
    acceptAction.destructive = NO;

    UIMutableUserNotificationAction *ignoreAction = [[UIMutableUserNotificationAction alloc] init];
    ignoreAction.identifier = FollowIgnoreIdentifier;
    ignoreAction.title = @"Ignore";
    ignoreAction.activationMode = UIUserNotificationActivationModeBackground;
    ignoreAction.destructive = YES;

    UIMutableUserNotificationCategory *category = [[UIMutableUserNotificationCategory alloc] init];
    category.identifier = FollowCategoryIdentifier;
    [category setActions: @[acceptAction, ignoreAction] forContext: UIUserNotificationActionContextDefault];
    [category setActions: @[acceptAction, ignoreAction] forContext: UIUserNotificationActionContextMinimal];

    NSSet *categories = [NSSet setWithObjects: category, nil];

    UIUserNotificationType types = UIUserNotificationTypeAlert | UIUserNotificationTypeSound |UIUserNotificationTypeBadge;
    UIUserNotificationSettings *settings = [UIUserNotificationSettings settingsForTypes: types categories: categories];
    [application registerUserNotificationSettings: settings];

    [application registerForRemoteNotifications];
}

- (void)
application:                (UIApplication *) application
handleActionWithIdentifier: (NSString *)      identifier
forRemoteNotification:      (NSDictionary *)  notification
completionHandler:          (void (^)())      completionHandler
{
    
    if ([identifier isEqualToString: FollowAcceptIdentifier]) {
        NSLog(@"Accept Tapped!");
    }
    
    completionHandler();
}

- (void)
application:                                      (UIApplication *) application
didRegisterForRemoteNotificationsWithDeviceToken: (NSData *)        deviceToken
{
    // Send device token to Layer so Layer can send pushes to this device.
    // For more information about Push, check out:
    // https://developer.layer.com/docs/ios/guides#push-notification
    NSError *error;
    BOOL success = [self.layerClient updateRemoteNotificationDeviceToken: deviceToken error: &error];
    if (success) {
        NSLog(@"Application did register for remote notifications: %@", deviceToken);

    } else {
        NSLog(@"Failed updating device token with error: %@", error);
    }
}

- (void)
application:                  (UIApplication *)                   application
didReceiveRemoteNotification: (NSDictionary *)                    userInfo
fetchCompletionHandler:       (void (^)(UIBackgroundFetchResult)) completionHandler
{
    NSError *error;
    BOOL success = [self.layerClient synchronizeWithRemoteNotification: userInfo completion:
        ^(LYRConversation * _Nullable conversation, LYRMessage * _Nullable message, NSError * _Nullable error) {
            if (conversation || message) {
                LYRMessagePart *messagePart = message.parts[0];

                if ([messagePart.MIMEType  isEqual: @"text/plain"]) {
                    NSLog(@"Pushed Message Contents: %@",[[NSString alloc] initWithData: messagePart.data encoding: NSUTF8StringEncoding]);

                } else if ([messagePart.MIMEType  isEqual: @"image/png"]){
                    NSLog(@"Pushed Message Contents was an image");
                }

                completionHandler(UIBackgroundFetchResultNewData);

            } else {
                completionHandler(error ? UIBackgroundFetchResultFailed : UIBackgroundFetchResultNoData);
            }
        }];

    if (success) {
        NSLog(@"Application did complete remote notification sync");

    } else {
        NSLog(@"Failed processing push notification with error: %@", error);
        completionHandler(UIBackgroundFetchResultNoData);
    }
}

- (void)
application:                         (UIApplication *) application
handleEventsForBackgroundURLSession: (NSString *)      identifier
completionHandler:                   (void (^)())      completionHandler
{
    [self.layerClient handleBackgroundContentTransfersForSession: identifier completion: ^(NSArray *changes, NSError *error) {
        NSLog(@"Background transfers finished with %lu change(s)", (unsigned long)changes.count);
        completionHandler();
    }];
}

#pragma mark - Layer Authentication Methods

- (void)
authenticateLayerWithUserID: (NSString *)                              userID
completion:                  (void (^)(BOOL success, NSError * error)) completion
{
    if (self.layerClient.authenticatedUser) {
        NSLog(@"Layer Authenticated as User %@", self.layerClient.authenticatedUser.userID);
        if (completion) completion(YES, nil);
        return;
    }

    // Authenticate with Layer. See "Quick Start - Authenticate" for more details:
    // https://developer.layer.com/docs/quick-start/ios#authenticate

    // 1. Request an authentication Nonce from Layer
    [self.layerClient requestAuthenticationNonceWithCompletion: ^(NSString *nonce, NSError *error) {
        if (!nonce) {
            if (completion) completion(NO, error);
            return;
        }

        // 2. Acquire identity Token from Layer Identity Service
        [self requestIdentityTokenForUserID: userID appID: [self.layerClient.appID absoluteString] nonce: nonce completion: ^(NSString *identityToken, NSError *error) {
            if (!identityToken) {
                if (completion) completion(NO, error);
                return;
            }

            // 3. Submit identity token to Layer for validation
            [self.layerClient authenticateWithIdentityToken: identityToken completion: ^(LYRIdentity *authenticatedUser, NSError *error) {
                if (authenticatedUser) {
                    if (completion) completion(YES, nil);
                    NSLog(@"Layer Authenticated as User: %@", authenticatedUser.userID);

                } else {
                    completion(NO, error);
                }
            }];
        }];
    }];
}

- (void)
requestIdentityTokenForUserID: (NSString *)                                       userID
appID:                         (NSString *)                                       appID
nonce:                         (NSString *)                                       nonce
completion:                    (void(^)(NSString *identityToken, NSError *error)) completion
{
    NSParameterAssert(userID);
    NSParameterAssert(appID);
    NSParameterAssert(nonce);
    NSParameterAssert(completion);

    NSURL *identityTokenURL = [NSURL URLWithString: @"https://layer-identity-provider.herokuapp.com/identity_tokens"];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL: identityTokenURL];
    request.HTTPMethod = @"POST";
    [request setValue: @"application/json" forHTTPHeaderField: @"Content-Type"];
    [request setValue: @"application/json" forHTTPHeaderField: @"Accept"];

    NSDictionary *parameters = @{ @"app_id": appID, @"user_id": userID, @"nonce": nonce };
    NSData *requestBody = [NSJSONSerialization dataWithJSONObject: parameters options: 0 error: nil];
    request.HTTPBody = requestBody;

    NSURLSessionConfiguration *sessionConfiguration = [NSURLSessionConfiguration ephemeralSessionConfiguration];
    NSURLSession *session = [NSURLSession sessionWithConfiguration: sessionConfiguration];
    [[session dataTaskWithRequest: request completionHandler: ^(NSData *data, NSURLResponse *response, NSError *error) {
        if (error) {
            completion(nil, error);
            return;
        }

        // Deserialize the response
        NSDictionary *responseObject = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
        if(![responseObject valueForKey: @"error"]) {
            NSString *identityToken = responseObject[@"identity_token"];
            completion(identityToken, nil);

        } else {
            NSString *domain = @"layer-identity-provider.herokuapp.com";
            NSInteger code = [responseObject[@"status"] integerValue];
            NSDictionary *userInfo = @{
                 NSLocalizedDescriptionKey: @"Layer Identity Provider Returned an Error.",
                 NSLocalizedRecoverySuggestionErrorKey: @"There may be a problem with your APPID."
            };

            NSError *error = [[NSError alloc] initWithDomain:domain code:code userInfo:userInfo];
            completion(nil, error);
        }
    }] resume];
}

#pragma - mark LYRClientDelegate Delegate Methods

- (void)
layerClient:                                (LYRClient *) client
didReceiveAuthenticationChallengeWithNonce: (NSString *)  nonce
{
    NSLog(@"Layer Client did recieve authentication challenge with nonce: %@", nonce);
}

- (void)
layerClient:             (LYRClient *) client
didAuthenticateAsUserID: (NSString *)  userID
{
    NSLog(@"Layer Client did recieve authentication nonce");
}

- (void) layerClientDidDeauthenticate: (LYRClient *) client
{
    NSLog(@"Layer Client did deauthenticate");
}

- (void)
layerClient:                         (LYRClient *) client
didFinishSynchronizationWithChanges: (NSArray *)   changes
{
    NSLog(@"Layer Client did finish synchronization");
}

- (void)
layerClient:                     (LYRClient *) client
didFailSynchronizationWithError: (NSError *)   error
{
    NSLog(@"Layer Client did fail synchronization with error: %@", error);
}

- (void)
layerClient:             (LYRClient *)    client
willAttemptToConnect:    (NSUInteger)     attemptNumber
afterDelay:              (NSTimeInterval) delayInterval
maximumNumberOfAttempts: (NSUInteger)     attemptLimit
{
    NSLog(@"Layer Client will attempt to connect");
}

- (void) layerClientDidConnect: (LYRClient *) client
{
    NSLog(@"Layer Client did connect");
}

- (void)
layerClient:                (LYRClient *) client
didLoseConnectionWithError: (NSError *)   error
{
    NSLog(@"Layer Client did lose connection with error: %@", error);
}

- (void) layerClientDidDisconnect: (LYRClient *) client
{
    NSLog(@"Layer Client did disconnect");
}

- (void)
layerClient:      (LYRClient *) client
objectsDidChange: (NSArray *)   changes
{
    for (LYRObjectChange *change in changes) {
        if (![change.object isKindOfClass: LYRConversation.class]) continue;

        if (change.type == LYRObjectChangeTypeUpdate && [change.property isEqualToString: @"metadata"]) {
            [[NSNotificationCenter defaultCenter] postNotificationName: ConversationMetadataDidChangeNotification object: change.object];
            continue;
        }

        if (change.type == LYRObjectChangeTypeUpdate && [change.property isEqualToString: @"participants"]) {
            [[NSNotificationCenter defaultCenter] postNotificationName: ConversationParticipantsDidChangeNotification object: change.object];
            continue;
        }

        if (change.type == LYRObjectChangeTypeDelete) {
            [[NSNotificationCenter defaultCenter] postNotificationName: ConversationDeletedNotification object: change.object];
            continue;
        }
    }
}

@end

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
UIAlertView *AlertWithError(NSError *error)
{
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Unexpected Error"
                                                  message:           error.localizedDescription
                                                  delegate:          nil
                                                  cancelButtonTitle: @"OK"
                                                  otherButtonTitles: nil                       ];
    [alertView show];
    return alertView;
}
#pragma clang diagnostic pop
