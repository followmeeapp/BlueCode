//
//  BlueCardLinkController.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueCardLinkController.h"

#import "BlueCardController.h"

#import "BlueApp.h"
#import "BlueClient.h"

#import "ServerRequest.h"

#import "CardObject.h"

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

- (void) viewDidLoad
{
    [super viewDidLoad];

    self.card = [CardObject objectForPrimaryKey: @(self.cardId)];

    if (self.card) {
        [self showCardController];

    } else {
        RLMRealm *realm = [RLMRealm defaultRealm];
        [realm beginWriteTransaction];

        CardObject *newCard = [[CardObject alloc] init];

        newCard.id = self.cardId;
        newCard.version = 0;

        [realm addOrUpdateObject: newCard];

        NSData *cardRequestPacket = [ServerRequest newCardRequestWithId: [APP_DELEGATE.blueClient nextRequestId] cardId: self.cardId version: 0];
        [APP_DELEGATE.blueClient sendRequest: cardRequestPacket];

        [realm commitWriteTransaction];

        self.card = newCard;

        self.token = [realm addNotificationBlock: ^(NSString *notification, RLMRealm *realm) {
            if (self.card.version > 0) [self showCardController];
        }];

        [self showLoadingCardController];

        self.timer = [NSTimer timerWithTimeInterval: 30.0
                              target:                self
                              selector:              @selector(timerDidFire:)
                              userInfo:              nil
                              repeats:               NO                     ];

        [[NSRunLoop currentRunLoop] addTimer: self.timer forMode: NSRunLoopCommonModes];
    }
}

- (void) timerDidFire: (NSTimer *) timer
{
    if (self.linkStatus == LoadingCard) {
        [self showLoadingFailedController];
    }

    [self.timer invalidate];
    self.timer = nil;
}

- (void) showLoadingCardController
{
    UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
    BlueCardController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueLoadingCardController"];

    self.linkStatus = LoadingCard;

    self.viewControllers = @[vc];
}

- (void) showLoadingFailedController
{
    UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
    BlueCardController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueLoadingFailedController"];

    self.linkStatus = LoadingFailed;

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
