//
//  AppController.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "AppController.h"

#import "Shared.h"

#import "DDTTYLogger.h"

#import "EOAnimation.h"

#import "AppClient.h"

#import "KSReachability.h"

#include "tweetnacl.h"

#define WEBSOCKET_PORT 8090

static const int ddLogLevel __attribute__((unused)) = LOG_LEVEL_DEBUG;

@interface RRAppController_ViewController : UIViewController @end
@implementation RRAppController_ViewController
- (UIStatusBarStyle) preferredStatusBarStyle { return UIStatusBarStyleLightContent; }
- (BOOL) prefersStatusBarHidden { return YES; }
@end

@interface AppController ()

@property (nonatomic, strong) KSReachability *internetReachability;
@property (nonatomic, assign) BOOL hasInternet;

@property (nonatomic, strong) AppClient *client;

@property (nonatomic, strong) NSString *deepLinkAction;
@property (nonatomic, strong) NSDictionary *deepLinkData;

@end

@implementation AppController

@synthesize window; // Mandatory, not sure why.

- (instancetype) init
{
    if (self = [super init]) {
        // Make UIWebView use the correct user agent so we get mobile versions of sites, just like Mobile Safari.
        NSString *userAgent = @"Mozilla/5.0 (iPhone; CPU iPhone OS 7_0 like Mac OS X) AppleWebKit/537.51.1 (KHTML, like Gecko) Version/7.0 Mobile/11A465 Safari/9537.53";
        [[NSUserDefaults standardUserDefaults] registerDefaults: @{ @"UserAgent" : userAgent }];

        self.session = [NSURLSession sessionWithConfiguration: [NSURLSessionConfiguration ephemeralSessionConfiguration]];
        
        [CALayer eo_patchCALayer];
    }

    return self;
}

#pragma mark - KSReachability

- (void) internetStatusDidChange: (BOOL) hasInternet
{
    DDLogInfo(@"Have Wi-Fi? %@.", hasInternet ? @"Yes" : @"No" );
    
    NSMethodSignature *signature = [AppController instanceMethodSignatureForSelector: _cmd];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature: signature];
    
    [invocation setTarget:self];
    [invocation setSelector:_cmd];
    [invocation setArgument:&hasInternet atIndex:2];
    
    return [self dispatchInvocationReturningVoid: invocation];
}

- (void) startReachability
{
    if (_internetReachability) return;
    
    _internetReachability = [KSReachability reachabilityToInternet];
    
    AppController *appController = self;
    
    _internetReachability.onInitializationComplete = ^(KSReachability *reachability) {
        NSLog(@"Internet reachability initialization complete. Reachability = %d. Flags = %x", reachability.reachable, reachability.flags);
        
        appController.hasInternet = reachability.reachable;
        
        [appController internetStatusDidChange: appController.hasInternet];
    };
    
    _internetReachability.onReachabilityChanged = ^(KSReachability* reachability) {
        NSLog(@"Internet reachability changed to %d. Flags = %x (blocks)", reachability.reachable, reachability.flags);
        
        appController.hasInternet = reachability.reachable;
        
        [appController internetStatusDidChange: appController.hasInternet];
    };
}

#pragma mark -

- (void) configureWebSocket
{
    // Set up the crypto.
    if (![IXKeychain hasValueForKey: @"secretKey"]) {
        unsigned char pk[crypto_box_PUBLICKEYBYTES];
        unsigned char sk[crypto_box_SECRETKEYBYTES];

        crypto_box_keypair(pk, sk);

        self.publicKey = [NSData dataWithBytes: &pk length: crypto_box_PUBLICKEYBYTES];
        self.secretKey = [NSData dataWithBytes: &sk length: crypto_box_SECRETKEYBYTES];

        [IXKeychain setSecureValue: self.publicKey forKey: @"publicKey"];
        [IXKeychain setSecureValue: self.secretKey forKey: @"secretKey"];

    } else {
        self.publicKey = [IXKeychain secureValueForKey: @"publicKey"];
        self.secretKey = [IXKeychain secureValueForKey: @"secretKey"];
    }

#ifdef XyDeveloperBuild
    self.serverKey = [NSData dataFromHexString: @"b22fee6b43906ed16cf1f146ca8a18c8501c2da32ead5fee3b09909f3019093f"];
    self.hostname = @"xygroup.local";
#endif

#ifdef XyBetaBuild
    self.serverKey = [NSData dataFromHexString: @"cd45f90cc76bf5723a31292bc1f523aa22d8212644b258e3e6d3241076eb0971"];
    self.hostname = @"199.19.87.92";
#endif

    [self startActivationServerClient];
}

// NOTE: Called by EOApplicationController as a template method.
- (void) configureLogging
{
    // Logging to the console.
    DDTTYLogger *ttyLogger = [DDTTYLogger sharedInstance];

    [DDLog addLogger: ttyLogger withLogLevel: LOG_LEVEL_DEBUG];

    [ttyLogger setColorsEnabled: YES];

    UIColor *fatal  = [UIColor blackColor];
    UIColor *error  = [UIColor colorWithRed:(255/255.0) green:(  6/255.0) blue:(  0/255.0) alpha:1.0];
    UIColor *warn   = [UIColor colorWithRed:(111/255.0) green:(168/255.0) blue:( 58/255.0) alpha:1.0];
    UIColor *notice = [UIColor colorWithRed:( 51/255.0) green:( 51/255.0) blue:( 51/255.0) alpha:1.0];
    UIColor *info   = [UIColor colorWithRed:( 64/255.0) green:( 64/255.0) blue:( 64/255.0) alpha:1.0];
    UIColor *debug  = [UIColor colorWithRed:( 92/255.0) green:( 92/255.0) blue:( 92/255.0) alpha:1.0];

    [ttyLogger setForegroundColor: fatal  backgroundColor: error forFlag: LOG_FLAG_FATAL];
    [ttyLogger setForegroundColor: error  backgroundColor: nil   forFlag: LOG_FLAG_ERROR];
    [ttyLogger setForegroundColor: warn   backgroundColor: nil   forFlag: LOG_FLAG_WARN];
    [ttyLogger setForegroundColor: notice backgroundColor: nil   forFlag: LOG_FLAG_NOTICE];
    [ttyLogger setForegroundColor: info   backgroundColor: nil   forFlag: LOG_FLAG_INFO];
    [ttyLogger setForegroundColor: debug  backgroundColor: nil   forFlag: LOG_FLAG_DEBUG];

    [ttyLogger setLogFormatter: self];
}

- (NSString *) formatLogMessage: (DDLogMessage *) logMessage
{
    // Suppress all that crap in the front of the debug console.
    return [NSString stringWithFormat:@" %@", logMessage->logMsg];
    return logMessage->logMsg;
}

- (void) createUserInterface
{
    //
    // This sets up the standard environment for our statechart app: a single,
    // generic root view controller with a full screen container view that has
    // a black background color.
    //
    CGRect frame = [[UIScreen mainScreen] bounds];
    window = [[UIWindow alloc] initWithFrame: frame];

    UIView *_rootView = [[UIView alloc] initWithFrame:CGRectZero];
    _rootView.backgroundColor = [UIColor blackColor];
    _rootView.opaque = YES;
    _rootView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    _rootView.restorationIdentifier = @"ROOT_VIEW_CONTAINER";
    self.rootView = _rootView;

    UIViewController *rootController = [[RRAppController_ViewController alloc] init];
    rootController.view = _rootView;

    window.rootViewController = rootController;

    [window makeKeyAndVisible];
}

- (void) createImagesDirectory
{
    NSArray *documentDirectoryPathList = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *parentDirectoryPath = [documentDirectoryPathList objectAtIndex: 0];
    NSString *imagesDirectoryPath = [parentDirectoryPath stringByAppendingString: @"/Images/"];
    NSLog(@"imagesDirectoryPath: %@", imagesDirectoryPath);

    BOOL shouldCreateDirectory = YES;
    BOOL isDirectory = NO;
    if ([[NSFileManager defaultManager] fileExistsAtPath: imagesDirectoryPath isDirectory: &isDirectory]) {
        if (isDirectory) shouldCreateDirectory = NO;
        else {
            // This is messed up. We need to delete the file that exists there now.
            NSError *error = nil;
            if (![[NSFileManager defaultManager] removeItemAtPath: imagesDirectoryPath error: &error]) {
                // Can't even delete the file! WTF
                DDLogFatal(@"FATAL: %@", error);
                abort();
            }
        }
    }

    if (shouldCreateDirectory) {
        NSError *error = nil;
        BOOL failed = ![[NSFileManager defaultManager] createDirectoryAtPath:       imagesDirectoryPath
                                                       withIntermediateDirectories: YES
                                                       attributes:                  nil
                                                       error:                       &error             ];

        if (failed) {
            // We can't run without the cache directory.
            DDLogFatal(@"FATAL: %@", error);
            abort();
        }
    }

    self.imagesDirectoryPath = imagesDirectoryPath;
}



#pragma mark - Dispatch Helpers

- (void) timeout: (NSTimer *) timer {} // for selector
- (void) dispatchTimeout: (NSTimer *) timer
{
    NSMethodSignature *signature = [AppController instanceMethodSignatureForSelector: @selector(timeout:)];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature: signature];

    [invocation setTarget: self];
    [invocation setSelector: @selector(timeout:)];
    [invocation setArgument: &timer atIndex: 2];

    return [self dispatchInvocationReturningVoid: invocation];
}

- (void) screenStateUpdate: (NSInteger) screenState {} // for selector
- (void) dispatchScreenStateUpdate: (NSInteger) screenState
{
    NSMethodSignature *signature = [AppController instanceMethodSignatureForSelector: @selector(timeout:)];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature: signature];
    
    [invocation setTarget: self];
    [invocation setSelector: @selector(screenStateUpdate:)];
    [invocation setArgument: &screenState atIndex: 2];
    
    return [self dispatchInvocationReturningVoid: invocation];
}

- (void) successResponse: (NSDictionary *) dict {} // for selector
- (void) dispatchSuccessResponse: (NSDictionary *) dict
{
    NSMethodSignature *signature = [AppController instanceMethodSignatureForSelector: @selector(timeout:)];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature: signature];
    
    [invocation setTarget: self];
    [invocation setSelector: @selector(successResponse:)];
    [invocation setArgument: &dict atIndex: 2];
    
    return [self dispatchInvocationReturningVoid: invocation];
}

- (void) errorResponse: (NSError *) error {} // for selector
- (void) dispatchErrorResponse: (NSError *) error
{
    NSMethodSignature *signature = [AppController instanceMethodSignatureForSelector: @selector(timeout:)];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature: signature];
    
    [invocation setTarget: self];
    [invocation setSelector: @selector(errorResponse:)];
    [invocation setArgument: &error atIndex: 2];
    
    return [self dispatchInvocationReturningVoid: invocation];
}

- (void)
handleDeepLinkWithDictionary: (NSDictionary *) params
error:                        (NSError *)      error
{
    if (error) {
        self.deepLinkAction = @"handleDeepLinkError";
        self.deepLinkData = @{ @"error": [error description] };
        
    } else {
        self.deepLinkAction = @"handleDeepLink";
        self.deepLinkData = params;
    }

    if (!_client) return;

    [self sendAction: self.deepLinkAction withArguments: self.deepLinkData];
}



#pragma mark - WebSocket Disptach Helpers

- (void) handleQuery: (NSDictionary *) query
{
    EOState *state = self.statechart.currentState;
    NSMutableArray *array = [NSMutableArray arrayWithObject: state];

    while ((state = state.superstate)) [array addObject: state];

    NSInteger idx = array.count;
    while (idx--) {
        state = array[idx];
        [state render: query];
    }
}

- (void) handleQueryError: (NSDictionary *) error
{
    EOState *state = self.statechart.currentState;
    NSMutableArray *array = [NSMutableArray arrayWithObject: state];

    while ((state = state.superstate)) [array addObject: state];

    NSInteger idx = array.count;
    while (idx--) {
        state = array[idx];
        [state renderError: error];
    }
}

- (void) handleConfiguration: (NSDictionary *) configuration
{
    NSMethodSignature *signature = [[self class] instanceMethodSignatureForSelector: _cmd];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature: signature];

    [invocation setTarget: self];
    [invocation setSelector: _cmd];
    [invocation setArgument: &configuration atIndex: 2];

    [self.statechart dispatchInvocation: invocation];
}

- (void) handleCommand: (NSDictionary *) command
{
    NSMethodSignature *signature = [[self class] instanceMethodSignatureForSelector: _cmd];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature: signature];

    [invocation setTarget: self];
    [invocation setSelector: _cmd];
    [invocation setArgument: &command atIndex: 2];

    [self.statechart dispatchInvocation: invocation];
}



#pragma mark Action Handling

- (void) sendAction: (NSString *) action
{
    [_client sendRequest: action];
}

- (void)
sendAction:    (NSString *)     action
withArguments: (NSDictionary *) args
{
    [_client sendRequest: action withOptions: args];
}



#pragma mark Dispatch helpers

- (void) dispatchInvocationReturningVoid: (NSInvocation *) invocation
{
    [self.statechart dispatchInvocation: invocation];
}



#pragma mark - Activation Server Client

- (void) startActivationServerClient
{
    NSString *hostname = self.hostname;
    NSData *serverKey = self.serverKey;

    self.client = [[AppClient alloc] initWithSecretKey: self.secretKey
                                     publicKey:         self.publicKey
                                     serverKey:         serverKey
                                     hostname:          hostname
                                     port:              WEBSOCKET_PORT];

    // The WebSocket will start automatically.
}

- (void) stopActivationServerClient
{
    [self.client stopWebSocket];
    self.client = nil;
}

#pragma mark - WebSocket notifications

- (void) webSocketStatusDidChange: (BOOL) hasWebSocket
{
    // Always send this to the server when our WebSocket connects, if we have it.
    if (hasWebSocket && self.deepLinkAction) {
        [self sendAction: self.deepLinkAction withArguments: self.deepLinkData];
    }

    NSMethodSignature *signature = [[self class] instanceMethodSignatureForSelector: _cmd];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature: signature];

    [invocation setTarget: self];
    [invocation setSelector: _cmd];
    [invocation setArgument: &hasWebSocket atIndex: 2];

    [self.statechart dispatchInvocation: invocation];
}

- (void) webSocketDidReceiveMessage: (NSDictionary *) dict
{
//    NSMethodSignature *signature = [[self class] instanceMethodSignatureForSelector: _cmd];
//    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature: signature];
//    
//    [invocation setTarget: self];
//    [invocation setSelector: _cmd];
//    [invocation setArgument: &dict atIndex: 2];
//    
//    [self.statechart dispatchInvocation: invocation];
}

@end
