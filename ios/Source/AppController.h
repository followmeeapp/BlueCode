//
//  AppController.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "EOApplicationController.h"

#import "EOLog.h"

// Make these globally available.
#import "RootState.h"
#import "TimeoutState.h"
#import "ScreenState.h"

#define APP_CONTROLLER ((AppController *)[[UIApplication sharedApplication] delegate])

@interface AppController : EOApplicationController <DDLogFormatter>

@property (nonatomic, strong) NSURLSession *session;

@property (nonatomic, strong) UIWindow *window;
@property (nonatomic, copy) NSString *imagesDirectoryPath;

// Shared Properties
@property (nonatomic, copy) NSString *link;
@property (nonatomic, copy) NSString *twitterHandle;
@property (nonatomic, strong) NSDictionary *followInfo;

// Server Client & Device Properties
@property (nonatomic, copy) NSString *hostname;
@property (nonatomic, copy) NSData *serverKey;

@property (nonatomic, copy) NSData *secretKey;
@property (nonatomic, copy) NSData *publicKey;
@property (nonatomic, copy) NSString *deviceType;
@property (nonatomic, copy) NSString *appType;

- (void) startActivationServerClient;
- (void) stopActivationServerClient;

// API
- (void) startReachability;
- (void) configureWebSocket;
- (void) configureLogging;
- (void) createUserInterface;
- (void) createImagesDirectory;

- (void) dispatchTimeout: (NSTimer *) timer;
- (void) dispatchScreenStateUpdate: (NSInteger) screenState;
- (void) dispatchSuccessResponse: (NSDictionary *) dict;
- (void) dispatchErrorResponse: (NSError *) error;

- (void) webSocketStatusDidChange: (BOOL) hasWebSocket;
- (void) webSocketDidReceiveMessage: (NSDictionary *) dict;

- (void)
handleDeepLinkWithDictionary: (NSDictionary *) params
error:                        (NSError *)      error;

- (void) handleQuery: (NSDictionary *) query;
- (void) handleQueryError: (NSDictionary *) error;
- (void) handleConfiguration: (NSDictionary *) configuration;
- (void) handleCommand: (NSDictionary *) command;

- (void) sendAction: (NSString *) action;

- (void)
sendAction:    (NSString *)     action
withArguments: (NSDictionary *) args;

// Config app
@property (weak, nonatomic) IBOutlet UIView *configViewPreview;

@end
