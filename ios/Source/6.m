//
//  5.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Shared.h"
#import "Follow-private.h"
#define STATE_NAME 6

#import <FBSDKCoreKit/FBSDKCoreKit.h>
#import <FBSDKLoginKit/FBSDKLoginKit.h>

static const int ddLogLevel __attribute__((unused)) = LOG_LEVEL_DEBUG;

@interface STATE : ScreenState
@end

@implementation STATE

- (NSString *) xibName
{
    return @"TempScreen";
}

#pragma mark - Actions

- (void) handleConfiguration: (NSDictionary *) configuration
{
    [self handled];

    if (![configuration[@"isFacebook"] boolValue]) return;

    NSString *userID = configuration[@"userID"];
    NSLog(@"userID: %@", userID);

    FBSDKAccessToken *token = [FBSDKAccessToken currentAccessToken];
    NSLog(@"token.userID: %@", token.userID);

    if (token && [token.userID isEqualToString: userID]) {
        [[[FBSDKGraphRequest alloc] initWithGraphPath: @"me" parameters: nil] startWithCompletionHandler: ^(FBSDKGraphRequestConnection *connection, id result, NSError *error) {
            if (result) {
                NSLog(@"Fetched User Information:%@", result);
                [APP_CONTROLLER sendAction: @"loadFacebookUser" withArguments: @{
                    @"facebookUser": @{
                        @"id": [result valueForKey: @"id"],
                        @"name": [result valueForKey: @"name"],
                    }
                }];

            } else {
                NSLog(@"User error: %@", [error localizedDescription]);
                [APP_CONTROLLER sendAction: @"loadFacebookUserError" withArguments: @{ @"error": [error description] }];
            }
        }];

    } else {
        [APP_CONTROLLER sendAction: @"facebookSessionError"];
    }
}

@end
