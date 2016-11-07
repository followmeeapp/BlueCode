//
//  BlueAnalytics.h
//  Follow
//
//  Created by Erich Ocean on 11/2/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

@interface BlueAnalytics : NSObject

- (void) digitsSignUpFail;
- (void) digitsSignUpSuccess;
- (void) digitsLoginFail;
- (void) digitsLoginSuccess;

- (void) adUnavailable;

- (void) discoverCardViaBluetooth: (NSInteger) cardId;
- (void) discoverCardViaLink: (NSInteger) cardId;
- (void) viewCardBySwiping: (NSInteger) cardId;
- (void) viewCardByTappingRow: (NSInteger) cardId;
- (void) viewOwnCard: (NSInteger) cardId;
- (void) viewCardViaCardLink: (NSInteger) cardId;
- (void) editCard: (NSInteger) cardId;
- (void) createCard;
- (void) viewTour: (NSString *) videoName;

- (void)
shareCard:  (NSInteger)  cardId
withMethod: (NSString *) activityType
URLString:  (NSString *) URLString
isUserCard: (BOOL)       isUserCard;

- (void)
openSocialNetwork: (NSString *) networkName
withUsername:      (NSString *) username
fromCard:          (NSInteger)  cardId;

- (void)
testSocialNetwork: (NSString *) networkName
withUsername:      (NSString *) username
fromCard:          (NSInteger)  cardId;

@end
