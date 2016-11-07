//
//  BlueAnalytics.m
//  Follow
//
//  Created by Erich Ocean on 11/2/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueAnalytics.h"

#import <Fabric/Fabric.h>
#import <Crashlytics/Crashlytics.h>

@implementation BlueAnalytics

- (void) digitsSignUpFail
{
    [Answers logSignUpWithMethod: @"Digits" success: @NO customAttributes: @{}];
}

- (void) digitsSignUpSuccess
{
    [Answers logSignUpWithMethod: @"Digits" success: @YES customAttributes: @{}];
}

- (void) digitsLoginFail
{
    [Answers logLoginWithMethod: @"Digits" success: @NO customAttributes: @{}];
}

- (void) digitsLoginSuccess
{
    [Answers logLoginWithMethod: @"Digits" success: @YES customAttributes: @{}];
}

- (void) adUnavailable
{
    [Answers logCustomEventWithName: @"Ad Unavailable"
             customAttributes:       @{}             ];
}

- (void) discoverCardViaBluetooth: (NSInteger) cardId
{
    [Answers logCustomEventWithName: @"Discovered Card"
             customAttributes:       @{ @"cardId" : @(cardId), @"isBLECard": @YES }];
}

- (void) discoverCardViaLink: (NSInteger) cardId
{
    [Answers logCustomEventWithName: @"Discovered Card"
             customAttributes:       @{ @"cardId" : @(cardId), @"cardLink": @YES }];
}

- (void) viewCardBySwiping: (NSInteger) cardId
{
    [Answers logContentViewWithName: @"View Card Screen"
             contentType:            @"Card"
             contentId:              [NSString stringWithFormat: @"%@", @(cardId)]
             customAttributes:       @{ @"swipped": @YES }                       ];
}

- (void) viewCardByTappingRow: (NSInteger) cardId
{
    [Answers logContentViewWithName: @"View Card Screen"
             contentType:            @"Card"
             contentId:              [NSString stringWithFormat: @"%@", @(cardId)]
             customAttributes:       @{ @"tappedRow": @YES }                     ];
}

- (void) viewOwnCard: (NSInteger) cardId
{
    [Answers logContentViewWithName: @"User Card Screen"
             contentType:            @"Card"
             contentId:              [NSString stringWithFormat: @"%@", @(cardId)]
             customAttributes:       @{}                                         ];
}

- (void) viewCardViaCardLink: (NSInteger) cardId
{
    [Answers logContentViewWithName: @"View Card Screen"
             contentType:            @"Card"
             contentId:              [NSString stringWithFormat: @"%@", @(cardId)]
             customAttributes:       @{ @"cardLink": @YES }                      ];
}

- (void) editCard: (NSInteger) cardId
{
    [Answers logContentViewWithName: @"Edit Card Screen"
             contentType:            @"Card"
             contentId:              [NSString stringWithFormat: @"%@", @(cardId)]
             customAttributes:       @{}                                         ];
}

- (void) createCard
{
    [Answers logContentViewWithName: @"Create Card Screen"
             contentType:            @"Card"
             contentId:              nil
             customAttributes:       @{}                 ];
}

- (void) viewTour: (NSString *) videoName
{
    [Answers logContentViewWithName: @"Tour"
             contentType:            videoName
             contentId:              nil
             customAttributes:       @{}      ];
}

- (void)
shareCard:  (NSInteger)  cardId
withMethod: (NSString *) activityType
URLString:  (NSString *) URLString
isUserCard: (BOOL)       isUserCard
{
    [Answers logShareWithMethod: activityType
             contentName:        URLString
             contentType:        nil
             contentId:          [NSString stringWithFormat: @"%@", @(cardId)]
             customAttributes:   @{ @"isUserCard": isUserCard ? @YES : @NO } ];
}

- (void)
openSocialNetwork: (NSString *) networkName
withUsername:      (NSString *) username
fromCard:          (NSInteger)  cardId
{
    [Answers logContentViewWithName: @"Open Social Network"
             contentType:            networkName
             contentId:              username
             customAttributes:       @{ @"cardId": [NSString stringWithFormat: @"%@", @(cardId)] }];
}

- (void)
testSocialNetwork: (NSString *) networkName
withUsername:      (NSString *) username
fromCard:          (NSInteger)  cardId
{
    [Answers logContentViewWithName: @"Test Social Network"
             contentType:            networkName
             contentId:              username
             customAttributes:       @{ @"cardId": [NSString stringWithFormat: @"%@", @(cardId)] }];
}

@end
