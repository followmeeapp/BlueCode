//
//  EditCardController.m
//  Follow
//
//  Created by Erich Ocean on 7/14/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "EditCardController.h"

#import "UserObject.h"

@interface EditCardController ()

@end

@implementation EditCardController

#pragma mark - DGTCompletionViewController delegate

- (void)
digitsAuthenticationFinishedWithSession: (DGTSession *) session
error:                                   (NSError *)    error
{
//    if (session) {
//        // Delay showing the contacts uploader while the Digits screen animates off-screen
//        //            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
//        //                [self uploadDigitsContactsForSession: session];
//        //            });
//
//        // Get the default Realm. You only need to do this once (per thread).
//        RLMRealm *realm = [RLMRealm defaultRealm];
//
//        UserObject *user = [[UserObject alloc] init];
//        NSString *phoneNumber = session.phoneNumber;
//        NSString *telephone = [phoneNumber substringWithRange: NSMakeRange(1, phoneNumber.length - 1)];
//        user.telephone = telephone;
//
//        // Add User object to Realm with transaction.
//        [realm beginWriteTransaction];
//        [realm addObject: user];
//        [realm commitWriteTransaction];
//
//        // TODO: Send Hello Request to the server.
//
//    } else {
//        NSLog(@"Authentication error: %@", error.localizedDescription);
//        // FIXME: Need to exit the Edit Card controller.
//        //            [APP_CONTROLLER sendAction: @"digitsSessionDidError" withArguments: @{ @"error": error.localizedDescription }];
//    }
}

@end
