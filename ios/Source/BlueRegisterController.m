//
//  BlueRegisterController.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueRegisterController.h"

#import <Crashlytics/Crashlytics.h>
#import <Realm/Realm.h>

#import "BlueApp.h"
#import "BlueModel.h"
#import "BlueClient.h"

#import "BlueCardController.h"

#import "ServerRequest.h"

#import "UserObject.h"
#import "NetworkObject.h"

@interface BlueRegisterController ()

@property (nonatomic, strong) id token;

@property (nonatomic, assign) BOOL isNewUser;

@property (nonatomic, assign) NSTimeInterval started;

@end

@implementation BlueRegisterController

- (void) viewDidLoad
{
    [super viewDidLoad];

    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(newUserNotification:) name: @"NewUser" object: nil];
    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(existingUserNotification:) name: @"ExistingUser" object: nil];

    self.started = [NSDate timeIntervalSinceReferenceDate];
}

- (void) viewDidDisappear: (BOOL) animated
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];

    [super viewDidDisappear: animated];
}

- (void) newUserNotification: (NSNotification *) note
{
    _isNewUser = YES;
}

- (void) existingUserNotification: (NSNotification *) note
{
    _isNewUser = NO;
}

- (void) didJoin: (NSNotification *) note
{
    NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
    NSTimeInterval delay = 0.0;

    if (now - self.started < 1.0) {
        delay = 1.0 - (now - self.started);
    }

    [APP_DELEGATE.blueAnalytics digitsLoginSuccess];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delay * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [APP_DELEGATE.window.rootViewController dismissViewControllerAnimated: YES completion: nil];
        [APP_DELEGATE startBlueBackup];
    });
}

#pragma mark - DGTCompletionViewController delegate

- (void)
digitsAuthenticationFinishedWithSession: (DGTSession *) session
error:                                   (NSError *)    error
{
    if (session) {
        NSInteger requestId = [APP_DELEGATE.blueClient nextRequestId];

        NSString *phoneNumber = session.phoneNumber;
        NSString *telephone = [phoneNumber substringWithRange: NSMakeRange(1, phoneNumber.length - 1)];

        NSString *digitsId = session.userID;
        NSString *email = session.emailAddress;
        BOOL isVerified = session.emailAddressIsVerified;

        NSData *request = [ServerRequest newJoinRequestWithId: requestId
                                         telephone:            telephone
                                         digitsId:             digitsId
                                         email:                email
                                         emailIsVerified:      isVerified];

        [APP_DELEGATE.blueClient sendRequest: request];

        if (self.shouldDismissNavigationController) {
            [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(didJoin:) name: @"DidJoin" object: nil];
            return;
        }

        RLMRealm *realm = [RLMRealm defaultRealm];

        self.token = [realm addNotificationBlock: ^(NSString *notification, RLMRealm *realm) {
            // Do we have a user yet?
            UserObject *user = [APP_DELEGATE.blueModel activeUser];

            if (user) {
                [self.token stop];
                self.token = nil; // Stop listening for updates.

                NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
                NSTimeInterval delay = 0.0;

                if (now - self.started < 1.5) {
                    delay = 1.5 - (now - self.started);
                }

                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delay * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                    [APP_DELEGATE startBlueBackup];

                    if (user.cardId == 0) {
                        [APP_DELEGATE.blueAnalytics digitsSignUpSuccess];

                        UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
                        BlueCardController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueCardController"];
                        vc.isEditableCard = YES;
                        vc.shouldEditUserCard = YES;

                        [APP_DELEGATE.blueAnalytics createCard];

                        [APP_DELEGATE prepareToEditUserCard];

                        NSMutableArray *vcs = [[self.navigationController viewControllers] mutableCopy];
                        NSUInteger lastVcIndex = [vcs count] - 1;
                        if (lastVcIndex > 0) {
                            [vcs replaceObjectAtIndex: lastVcIndex withObject: vc];
                            [self.navigationController setViewControllers: vcs animated: YES];
                        }

                    } else {
                        [APP_DELEGATE.blueAnalytics digitsLoginSuccess];

                        [APP_DELEGATE startBluetoothAdvertising];
                        [self.navigationController popViewControllerAnimated: YES];
                    }
                });
            }
        }];

    } else {
        if (self.shouldDismissNavigationController) {
            [APP_DELEGATE.blueAnalytics digitsLoginFail];

        } else {
            [APP_DELEGATE.blueAnalytics digitsSignUpFail];
        }

        NSLog(@"Authentication error: %@", error.localizedDescription);
        AlertWithMessage(error.localizedDescription);

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
            if (self.shouldDismissNavigationController) {
                [APP_DELEGATE.window.rootViewController dismissViewControllerAnimated: YES completion: nil];

            } else {
                [self.navigationController popViewControllerAnimated: YES];
            }
        });
    }
}

@end
