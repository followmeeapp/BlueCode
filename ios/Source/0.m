//
//  0.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Shared.h"
#import "Follow-private.h"
#define STATE_NAME 0

#import <DigitsKit/DigitsKit.h>

static const int ddLogLevel __attribute__((unused)) = LOG_LEVEL_DEBUG;

@interface STATE : ScreenState

@property (nonatomic, strong) UIButton *loginButton;

@end

@implementation STATE

SUBSTATES( 1, 2 )

- (NSString *) xibName
{
    return @"LaunchScreen";
}

- (void) enterState
{
    [super enterState];

    UIView *contentView = self.contentView;
    CGPoint center = contentView.center;
    
    self.loginButton = [[UIButton alloc] init];
    [self.loginButton setTitle: @"Login with phone number" forState: UIControlStateNormal];
    [self.loginButton setTitleColor: [UIColor whiteColor] forState: UIControlStateNormal];
    [self.loginButton addTarget: self action: @selector(didTapLoginButton:) forControlEvents: UIControlEventTouchUpInside];
    [self.loginButton sizeToFit];

    center.y = contentView.frame.size.height - 100;
    self.loginButton.center = center;

//    [self styleButton: self.loginButton];

    [contentView addSubview: self.loginButton];

    [[Digits sharedInstance] logOut];
}

- (void) exitState
{
    [self.loginButton removeFromSuperview];
    self.loginButton = nil;
}

//- (void) uploadDigitsContactsForSession: (DGTSession *) session
//{
//    DGTContacts *digitsContacts = [[DGTContacts alloc] initWithUserSession:session];
//    
//    [digitsContacts startContactsUploadWithCompletion: ^(DGTContactsUploadResult *result, NSError *error) {
//        if (!result) {
//            // User tapped "Not now".
//            [APP_CONTROLLER sendAction: @"doNotFindFriends"];
//
//        } else {
//            [APP_CONTROLLER sendAction: @"contactUploadProgress" withArguments: @{
//                @"totalContacts": @(result.totalContacts),
//                @"numberOfUploadedContacts": @(result.numberOfUploadedContacts)
//            }];
//
//            [self findDigitsFriendsForSession: session];
//        }
//    }];
//}
//
//- (void) findDigitsFriendsForSession: (DGTSession *) session
//{
//    DGTContacts *digitsContacts = [[DGTContacts alloc] initWithUserSession: session];
//
//    // looking up friends happens in batches. Pass nil as cursor to get the first batch.
//    [digitsContacts lookupContactMatchesWithCursor: nil completion: ^(NSArray *matches, NSString *nextCursor, NSError *error) {
//        // If nextCursor is not nil, you can continue to call lookupContactMatchesWithCursor: to retrieve even more friends.
//        // Matches contain instances of DGTUser. Use DGTUser's userId to lookup users in your own database.
//        NSLog(@"Friends:");
//        for (DGTUser *digitsUser in matches) {
//            NSLog(@"Digits ID: %@", digitsUser.userID);
//        }
//        // Show the alert on the main thread
//        dispatch_async(dispatch_get_main_queue(), ^{
//            NSString *msg = [NSString stringWithFormat:@"%zd friends found!", matches.count];
//            UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Lookup complete"
//                                                            message:msg
//                                                           delegate:nil
//                                                  cancelButtonTitle:@"OK"
//                                                  otherButtonTitles:nil];
//            [alert show];
//        });
//    }];
//}

#pragma mark - Actions

- (IBAction) didTapLoginButton: (UIButton *) sender
{
    [self handled];
    
    DGTAuthenticationConfiguration *configuration = [[DGTAuthenticationConfiguration alloc] initWithAccountFields: DGTAccountFieldsEmail];

    [[Digits sharedInstance] authenticateWithViewController: nil configuration: configuration completion: ^(DGTSession *session, NSError *error) {
        if (session) {
            // Delay showing the contacts uploader while the Digits screen animates off-screen
//            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
//                [self uploadDigitsContactsForSession: session];
//            });

            dispatch_async(dispatch_get_main_queue(), ^{
                NSString *emailAddress = session.emailAddress;

                [APP_CONTROLLER sendAction: @"digitsSessionDidAuthenticate" withArguments: @{
                    @"digits": @{
                        @"userID": session.userID,
                        @"emailAddress": emailAddress? emailAddress : [NSNull null],
                        @"emailAddressIsVerified": session.emailAddressIsVerified ? @YES : @NO,
                        @"phoneNumber": session.phoneNumber,
                    }
                }];

                NSString *msg = [NSString stringWithFormat:@"Email address: %@, phone number: %@", session.emailAddress, session.phoneNumber];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                UIAlertView *alert = [[UIAlertView alloc] initWithTitle:     @"You are logged in!"
                                                          message:           msg
                                                          delegate:          nil
                                                          cancelButtonTitle: @"OK"
                                                          otherButtonTitles: nil                 ];
                [alert show];
#pragma clang diagnostic pop
            });

        } else {
            NSLog(@"Authentication error: %@", error.localizedDescription);
            [APP_CONTROLLER sendAction: @"digitsSessionDidError" withArguments: @{ @"error": error.localizedDescription }];
        }
    }];
}

@end
