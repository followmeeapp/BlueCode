//
//  BlueCardLinkController.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueCardLinkController.h"

#import <Crashlytics/Crashlytics.h>
#import <SVProgressHUD/SVProgressHUD.h>

#import "BlueCardController.h"

#import "BlueApp.h"
#import "BlueClient.h"
#import "BlueModel.h"

#import "ServerRequest.h"

#import "CardObject.h"
#import "DeviceObject.h"

typedef NS_ENUM(NSInteger, LinkStatus) {
    Unknown = 0,
    LoadingCard,
    LoadingFailed,
    HaveCard,
};

@interface BlueCardLinkController ()

@property (nonatomic, assign) LinkStatus linkStatus;

@property (nonatomic, strong) CardObject *card;

@property (nonatomic, strong) RLMNotificationToken *token;

@property (nonatomic, strong) NSTimer *timer;

@end

@implementation BlueCardLinkController

- (void) configure
{
    self.card = [CardObject objectForPrimaryKey: @(self.cardId)];

    if (self.card) {
        [self showCardController];

    } else {
        RLMRealm *realm = [RLMRealm defaultRealm];
        [realm beginWriteTransaction];

        [APP_DELEGATE.blueAnalytics discoverCardViaLink: self.cardId];

        CardObject *newCard = [[CardObject alloc] init];

        newCard.id = self.cardId;
        newCard.version = 0;
        newCard.fullName = self.fullName;
        newCard.location = self.location;

        [realm addOrUpdateObject: newCard];

        DeviceObject *device = [APP_DELEGATE.blueModel activeDevice];

        NSMutableArray *visibleCards = [[[device.visibleCards reverseObjectEnumerator] allObjects] mutableCopy];
        [visibleCards addObject: @(self.cardId)];
        [device updateVisibleCards: visibleCards hiddenCards: @[]];

        [realm addOrUpdateObject: device];

        [realm commitWriteTransaction];

        NSData *cardRequestPacket = [ServerRequest newCardRequestWithId: [APP_DELEGATE.blueClient nextRequestId] cardId: self.cardId version: 0];
        [APP_DELEGATE.blueClient sendRequest: cardRequestPacket];

        self.card = newCard;

        self.token = [realm addNotificationBlock: ^(NSString *notification, RLMRealm *realm) {
            if (self.card.version > 0) [self showCardController];
        }];

        [self showLoadingCardController];

        self.timer = [NSTimer timerWithTimeInterval: 7.0
                              target:                self
                              selector:              @selector(timerDidFire:)
                              userInfo:              nil
                              repeats:               NO                     ];

        [[NSRunLoop currentRunLoop] addTimer: self.timer forMode: NSRunLoopCommonModes];
    }
}

- (void) dealloc
{
    [self.timer invalidate];
    self.timer = nil;
}

- (void) timerDidFire: (NSTimer *) timer
{
    if (self.linkStatus == LoadingCard) {
        [SVProgressHUD setDefaultStyle: SVProgressHUDStyleCustom];
        [SVProgressHUD setDefaultMaskType: SVProgressHUDMaskTypeBlack];
        [SVProgressHUD setForegroundColor: [UIColor blackColor]];
        [SVProgressHUD setBackgroundColor: [UIColor whiteColor]];

        [SVProgressHUD showSuccessWithStatus: @"Loading failed."];

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [SVProgressHUD dismiss];
        });

        [self dismissViewControllerAnimated: YES completion: nil];
    }

    [self.timer invalidate];
    self.timer = nil;
}

- (void) showLoadingCardController
{
    UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
    BlueCardController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueLoadingCardController"];

    vc.card = self.card;

    self.linkStatus = LoadingCard;

    self.viewControllers = @[vc];
}

- (void) showCardController
{
    NSAssert(self.card, @"self.card must be defined before this method is called");

    if (self.token) {
        [self.token stop];
        self.token = nil;
    }

    UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
    BlueCardController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueCardController"];

    self.linkStatus = HaveCard;

    vc.isCardLink = YES;
    vc.card = self.card;

    self.viewControllers = @[vc];
}

#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
//- (void)
//prepareForSegue: (UIStoryboardSegue *) segue
//sender:          (id)                  sender
//{
//    // Get the new view controller using [segue destinationViewController].
//    // Pass the selected object to the new view controller.
//}

@end
