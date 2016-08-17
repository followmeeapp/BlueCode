//
//  5.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Shared.h"
#import "Follow-private.h"
#define STATE_NAME 5

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

    if (![configuration[@"isTwitter"] boolValue]) return;
    
    NSString *userID = configuration[@"userID"];

    TWTRSessionStore *store = [[Twitter sharedInstance] sessionStore];
    TWTRSession *session = [store sessionForUserID: userID];

    if (session) {
        [[TWTRAPIClient clientWithCurrentUser] loadUserWithID: session.userID completion: ^(TWTRUser *user, NSError *error) {
            if (user) {
                [APP_CONTROLLER sendAction: @"loadTwitterUser" withArguments: @{
                    @"twitterUser": @{
                        @"userID": user.userID,
                        @"name": user.name,
                        @"screenName": user.screenName,
                        @"isVerified": [NSNumber numberWithBool: user.isVerified],
                        @"isProtected": [NSNumber numberWithBool: user.isProtected],
                        @"profileImageURL": user.profileImageURL,
                        @"profileImageMiniURL": user.profileImageMiniURL,
                        @"profileImageLargeURL": user.profileImageLargeURL,
                        @"formattedScreenName": user.formattedScreenName,
                        @"profileURL": [((NSURL *)user.profileURL) absoluteString],
                    }
                }];

            } else {
                NSLog(@"User error: %@", [error localizedDescription]);
                [APP_CONTROLLER sendAction: @"loadTwitterUserError" withArguments: @{ @"error": [error description] }];
            }
        }];

    } else {
        [APP_CONTROLLER sendAction: @"twitterSessionError"];
    }
}

@end
