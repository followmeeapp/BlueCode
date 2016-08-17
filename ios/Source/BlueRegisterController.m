//
//  BlueRegisterController.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueRegisterController.h"

#import <Realm/Realm.h>

#import "BlueApp.h"
#import "BlueModel.h"
#import "BlueClient.h"

#import "BlueCardEditController.h"

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

#pragma mark - DGTCompletionViewController delegate

- (void)
digitsAuthenticationFinishedWithSession: (DGTSession *) session
error:                                   (NSError *)    error
{
    if (session) {
        NSInteger requestId = [APP_DELEGATE.blueClient nextRequestId];

        NSString *phoneNumber = session.phoneNumber;
        NSString *telephone = [phoneNumber substringWithRange: NSMakeRange(1, phoneNumber.length - 1)];

        NSData *request = [ServerRequest newJoinRequestWithId: requestId telephone: telephone];
        [APP_DELEGATE.blueClient sendRequest: request];

        RLMRealm *realm = [RLMRealm defaultRealm];

        self.token = [realm addNotificationBlock: ^(NSString *notification, RLMRealm *realm) {
            // Do we have a user yet?
            UserObject *user = [APP_DELEGATE.blueModel activeUser];

//            if (user && user.cardId == 0) {
//                [APP_DELEGATE.blueClient sendRequest: [ServerRequest newCreateCardRequestWithId: [APP_DELEGATE.blueClient nextRequestId] properties: @{
//                    @"fullName": [NSString stringWithFormat: @"Erich Ocean #%@", @(user.activeDeviceId)],
//                    @"location": @"Palmdale, CA",
//                    @"location": [NSString stringWithFormat: @"%@", user.activeDeviceUUID],
//                    @"networks": @{
//                        @(FacebookType): @"erich.ocean",
//                        @(TwitterType): @"erichocean",
//                        @(InstagramType): @"erichocean2",
//                    }
//                }]];
//
//            } else
                if (user) {
                [self.token stop];
                self.token = nil; // Stop listening for updates.

                NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
                NSTimeInterval delay = 0.0;

                if (now - self.started < 1.0) {
                    delay = 1.0 - (now - self.started);
                }

                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delay * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                    if (user.cardId == 0) {
                        UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
                        BlueCardEditController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueCardEditController"];

                        NSMutableArray *vcs = [[self.navigationController viewControllers] mutableCopy];
                        NSUInteger lastVcIndex = [vcs count] - 1;
                        if (lastVcIndex > 0) {
                            [vcs replaceObjectAtIndex: lastVcIndex withObject: vc];
                            [self.navigationController setViewControllers: vcs animated: YES];
                        }

//                        if (self.isNewUser) {
//                            // We don't have a card, so transition to the create card screen.
//                            AlertWithMessage(@"New user without card not implemented.");
//
//                        } else {
//                            // We don't have a card, so transition to the create card screen.
//                            AlertWithMessage(@"Existing user without card not implemented.");
//                        }

                    } else {
                        // We do have a card (and probably other stuff, sections and what not.
                        // We need to load cards referenced by our sections (our own card will
                        // already be here) and then transition back to the main screen.
                        //                    AlertWithMessage(@"Existing user with card not implemnted.");
                        
                        [self.navigationController popViewControllerAnimated: YES];
                    }
                });
            }
        }];

    } else {
        NSLog(@"Authentication error: %@", error.localizedDescription);
        AlertWithMessage(error.localizedDescription);
        // FIXME: Need to exit the Register controller.
    }
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
