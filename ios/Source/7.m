//
//  7.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Shared.h"
#import "Follow-private.h"
#define STATE_NAME 7

//#import "InstagramKit.h"

static const int ddLogLevel __attribute__((unused)) = LOG_LEVEL_DEBUG;

@interface STATE : ScreenState
@end

@implementation STATE

- (void) enterState
{
    [super enterState];
    
//    self.webView.delegate = (id<UIWebViewDelegate>)self;
//    
//    NSURL *authURL = [[InstagramEngine sharedEngine] authorizationURL];
//    [self.webView loadRequest: [NSURLRequest requestWithURL: authURL]];
}

#pragma mark - UIWebViewDelegate

- (BOOL)
webView:                    (UIWebView *)             webView
shouldStartLoadWithRequest: (NSURLRequest *)          request
navigationType:             (UIWebViewNavigationType) navigationType
{
//    NSError *error;
//
//    NSString *URLString = [request.URL absoluteString];
//    DDLogDebug(@"URLString: %@", URLString);
//
//    if ([[InstagramEngine sharedEngine] receivedValidAccessTokenFromURL: request.URL error: &error]) {
//        NSArray *components = [URLString componentsSeparatedByString: @"access_token="];
//
//        if (components.count > 1) {
//            NSString *accessToken = [components lastObject];
//            DDLogInfo(@"INSTAGRAM ACCESS TOKEN = %@", accessToken);
//
//            [[InstagramEngine sharedEngine] setAccessToken: accessToken];
//
//            [[InstagramEngine sharedEngine] getSelfUserDetailsWithSuccess: ^(InstagramUser *userDetail) {
//                [APP_CONTROLLER sendAction: @"loadInstagramUser" withArguments: @{
//                    @"instagramUser": @{
//                        @"username": userDetail.username,
//                        @"fullName": userDetail.fullName,
//                        @"profilePictureURL": [userDetail.profilePictureURL absoluteString],
//                        @"bio": userDetail.bio,
//                        @"website": [userDetail.website absoluteString],
//                        @"mediaCount": @(userDetail.mediaCount),
//                        @"followsCount": @(userDetail.followsCount),
//                        @"followedByCount": @(userDetail.followedByCount),
//                    }
//                }];
//
//            } failure: ^(NSError *error, NSInteger status) {
//                NSLog(@"User error: %@", error.description);
//                [APP_CONTROLLER sendAction: @"loadInstagramUserError" withArguments: @{ @"error": error.description }];
//            }];
//
//        } else {
//            NSLog(@"Failed to find access_token");
//            [APP_CONTROLLER sendAction: @"loadInstagramUserError" withArguments: @{ @"error": @"Failed to find access_token" }];
//        }
//
//        return NO;
//    }
//
//    if (error) {
//        NSLog(@"IG Error %@", error.description);
//        [APP_CONTROLLER sendAction: @"loadInstagramUserError" withArguments: @{ @"error": error.description }];
//    }

    return YES;
}

- (void)
webView:              (UIWebView *) webView
didFailLoadWithError: (NSError *)   error
{
    [self handled];
    
    DDLogError(@"Failed to load Instagram login URL: %@", error);
}

@end
