//
//  ConversationViewController.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "ConversationViewController.h"

#import "Follow.h"
#import "ConversationDetailViewController.h"
#import "MediaViewController.h"
#import "ParticipantTableViewController.h"

static NSDateFormatter *FollowShortTimeFormatter()
{
    static NSDateFormatter *dateFormatter;
    if (!dateFormatter) {
        dateFormatter = [[NSDateFormatter alloc] init];
        dateFormatter.timeStyle = NSDateFormatterShortStyle;
    }
    return dateFormatter;
}

static NSDateFormatter *FollowDayOfWeekDateFormatter()
{
    static NSDateFormatter *dateFormatter;
    if (!dateFormatter) {
        dateFormatter = [[NSDateFormatter alloc] init];
        dateFormatter.dateFormat = @"EEEE"; // Tuesday
    }
    return dateFormatter;
}

static NSDateFormatter *FollowRelativeDateFormatter()
{
    static NSDateFormatter *dateFormatter;
    if (!dateFormatter) {
        dateFormatter = [[NSDateFormatter alloc] init];
        dateFormatter.dateStyle = NSDateFormatterMediumStyle;
        dateFormatter.doesRelativeDateFormatting = YES;
    }
    return dateFormatter;
}

static NSDateFormatter *FollowThisYearDateFormatter()
{
    static NSDateFormatter *dateFormatter;
    if (!dateFormatter) {
        dateFormatter = [[NSDateFormatter alloc] init];
        dateFormatter.dateFormat = @"E, MMM dd,"; // Sat, Nov 29,
    }
    return dateFormatter;
}

static NSDateFormatter *FollowDefaultDateFormatter()
{
    static NSDateFormatter *dateFormatter;
    if (!dateFormatter) {
        dateFormatter = [[NSDateFormatter alloc] init];
        dateFormatter.dateFormat = @"MMM dd, yyyy,"; // Nov 29, 2013,
    }
    return dateFormatter;
}

typedef NS_ENUM(NSInteger, FollowDateProximity) {
    FollowDateProximityToday,
    FollowDateProximityYesterday,
    FollowDateProximityWeek,
    FollowDateProximityYear,
    FollowDateProximityOther,
};

static FollowDateProximity FollowProximityToDate(NSDate *date)
{
    NSCalendar *calendar = [NSCalendar currentCalendar];
    NSDate *now = [NSDate date];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    NSCalendarUnit calendarUnits = NSEraCalendarUnit | NSYearCalendarUnit | NSWeekOfMonthCalendarUnit | NSMonthCalendarUnit | NSDayCalendarUnit;
#pragma GCC diagnostic pop
    NSDateComponents *dateComponents = [calendar components: calendarUnits fromDate: date];
    NSDateComponents *todayComponents = [calendar components: calendarUnits fromDate: now];

    if (dateComponents.day == todayComponents.day &&
        dateComponents.month == todayComponents.month &&
        dateComponents.year == todayComponents.year &&
        dateComponents.era == todayComponents.era) {
        return FollowDateProximityToday;
    }

    NSDateComponents *componentsToYesterday = [NSDateComponents new];
    componentsToYesterday.day = -1;
    NSDate *yesterday = [calendar dateByAddingComponents: componentsToYesterday toDate: now options: 0];
    NSDateComponents *yesterdayComponents = [calendar components: calendarUnits fromDate: yesterday];

    if (dateComponents.day == yesterdayComponents.day &&
        dateComponents.month == yesterdayComponents.month &&
        dateComponents.year == yesterdayComponents.year &&
        dateComponents.era == yesterdayComponents.era) {
        return FollowDateProximityYesterday;
    }

    if (dateComponents.weekOfMonth == todayComponents.weekOfMonth &&
        dateComponents.month == todayComponents.month &&
        dateComponents.year == todayComponents.year &&
        dateComponents.era == todayComponents.era) {
        return FollowDateProximityWeek;
    }

    if (dateComponents.year == todayComponents.year &&
        dateComponents.era == todayComponents.era) {
        return FollowDateProximityYear;
    }

    return FollowDateProximityOther;
}

NSString *const ConversationViewControllerAccessibilityLabel = @"Conversation View Controller";
NSString *const DetailsButtonAccessibilityLabel = @"Details Button";
NSString *const DetailsButtonLabel = @"Details";

@interface ConversationViewController () <ConversationDetailViewControllerDelegate, ATLParticipantTableViewControllerDelegate>

@end

@implementation ConversationViewController

- (void) viewDidLoad
{
    [super viewDidLoad];

    self.view.accessibilityLabel = ConversationViewControllerAccessibilityLabel;
    self.dataSource = self;
    self.delegate = self;

    if (self.conversation) [self addDetailsButton];

    [self configureUserInterfaceAttributes];
    [self registerNotificationObservers];
}

- (void) viewWillAppear: (BOOL) animated
{
    [super viewWillAppear: animated];
    [self configureTitle];
}

- (void) viewDidAppear: (BOOL) animated
{
    [super viewDidAppear: animated];

    if (!self.view.isFirstResponder) [self.view becomeFirstResponder];
}

- (void) viewWillDisappear: (BOOL) animated
{
    [super viewWillDisappear: animated];

    if (![self isMovingFromParentViewController]) [self.view resignFirstResponder];
}

- (void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
}

#pragma mark - Accessors

- (void)setConversation: (LYRConversation *) conversation
{
    [super setConversation: conversation];
    [self configureTitle];
}

#pragma mark - ATLConversationViewControllerDelegate

/**
 Atlas - Informs the delegate of a successful message send. Atlas Messenger adds a `Details`
 button to the navigation bar if this is the first message sent within a new conversation.
 */
- (void)
conversationViewController: (ATLConversationViewController *) viewController
didSendMessage:             (LYRMessage *)                    message
{
    [self addDetailsButton];
}

/**
 Atlas - Informs the delegate that a message failed to send. Atlas messeneger display an alert
 view to inform the user of the failure.
 */
- (void)
conversationViewController: (ATLConversationViewController *) viewController
didFailSendingMessage:      (LYRMessage *)                    message
error:                      (NSError *)                       error
{
    NSLog(@"Message Send Failed with Error: %@", error);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Messaging Error"
                                                  message:           error.localizedDescription
                                                  delegate:          nil
                                                  cancelButtonTitle: @"OK"
                                                  otherButtonTitles: nil                       ];
#pragma clang diagnostic pop

    [alertView show];
}

/**
 Atlas - Informs the delegate that a message was selected. Atlas messenger presents an
 `ATLImageViewController` if the message contains an image.
 */
- (void)
conversationViewController: (ATLConversationViewController *) viewController
didSelectMessage:           (LYRMessage *)                    message
{
    LYRMessagePart *messagePart = ATLMessagePartForMIMEType(message, ATLMIMETypeImageJPEG);
    if (messagePart) {
        [self presentMediaViewControllerWithMessage: message];
        return;
    }

    messagePart = ATLMessagePartForMIMEType(message, ATLMIMETypeImagePNG);
    if (messagePart) {
        [self presentMediaViewControllerWithMessage: message];
        return;
    }

    messagePart = ATLMessagePartForMIMEType(message, ATLMIMETypeImageGIF);
    if (messagePart) {
        [self presentMediaViewControllerWithMessage: message];
        return;
    }

    messagePart = ATLMessagePartForMIMEType(message, ATLMIMETypeVideoMP4);
    if (messagePart) {
        [self presentMediaViewControllerWithMessage: message];
        return;
    }
}

- (void) presentMediaViewControllerWithMessage: (LYRMessage *) message
{
    MediaViewController *imageViewController = [[MediaViewController alloc] initWithMessage: message];
    UINavigationController *controller = [[UINavigationController alloc] initWithRootViewController: imageViewController];
    [self.navigationController presentViewController: controller animated: YES completion: nil];
}

#pragma mark - ATLConversationViewControllerDataSource

/**
 Atlas - Returns an object conforming to the `ATLParticipant` protocol whose `userID`
 property matches the supplied identity.
 */
- (id <ATLParticipant>)
conversationViewController: (ATLConversationViewController *) conversationViewController
participantForIdentity:     (nonnull LYRIdentity *)           identity
{
    return (id <ATLParticipant>)identity;
}

/**
 Atlas - Returns an `NSAttributedString` object for a given date. The format of this
 string can be configured to whatever format an application wishes to display.
 */
- (NSAttributedString *)
conversationViewController:       (ATLConversationViewController *) conversationViewController
attributedStringForDisplayOfDate: (NSDate *)                        date
{
    NSDateFormatter *dateFormatter;

    FollowDateProximity dateProximity = FollowProximityToDate(date);
    switch (dateProximity) {
        case FollowDateProximityToday:
        case FollowDateProximityYesterday:
            dateFormatter = FollowRelativeDateFormatter();
            break;

        case FollowDateProximityWeek:
            dateFormatter = FollowDayOfWeekDateFormatter();
            break;

        case FollowDateProximityYear:
            dateFormatter = FollowThisYearDateFormatter();
            break;

        case FollowDateProximityOther:
            dateFormatter = FollowDefaultDateFormatter();
            break;
    }

    NSString *dateString = [dateFormatter stringFromDate: date];
    NSString *timeString = [FollowShortTimeFormatter() stringFromDate: date];

    NSMutableAttributedString *dateAttributedString = [[NSMutableAttributedString alloc] initWithString: [NSString stringWithFormat:@"%@ %@", dateString, timeString]];

    [dateAttributedString addAttribute: NSForegroundColorAttributeName value: [UIColor grayColor] range: NSMakeRange(0, dateAttributedString.length)];
    [dateAttributedString addAttribute: NSFontAttributeName value: [UIFont systemFontOfSize: 11] range: NSMakeRange(0, dateAttributedString.length)];
    [dateAttributedString addAttribute: NSFontAttributeName value: [UIFont boldSystemFontOfSize: 11] range: NSMakeRange(0, dateString.length)];

    return dateAttributedString;
}

/**
 Atlas - Returns an `NSAttributedString` object for given recipient state. The state
 string will only be displayed below the latest message that was sent by the currently
 authenticated user.
 */
- (NSAttributedString *)
conversationViewController:                  (ATLConversationViewController *) conversationViewController
attributedStringForDisplayOfRecipientStatus: (NSDictionary *)                  recipientStatus
{
    NSMutableDictionary *mutableRecipientStatus = [recipientStatus mutableCopy];
    if ([mutableRecipientStatus valueForKey: APP_DELEGATE.layerClient.authenticatedUser.userID]) {
        [mutableRecipientStatus removeObjectForKey: APP_DELEGATE.layerClient.authenticatedUser.userID];
    }

    NSString *statusString = [NSString new];
    if (mutableRecipientStatus.count > 1) {
        __block NSUInteger readCount = 0;
        __block BOOL delivered = NO;
        __block BOOL sent = NO;
        __block BOOL pending = NO;

        [mutableRecipientStatus enumerateKeysAndObjectsUsingBlock: ^(NSString *userID, NSNumber *statusNumber, BOOL *stop) {
            LYRRecipientStatus status = statusNumber.integerValue;
            switch (status) {
                case LYRRecipientStatusInvalid:
                    break;

                case LYRRecipientStatusPending:
                    pending = YES;
                    break;

                case LYRRecipientStatusSent:
                    sent = YES;
                    break;

                case LYRRecipientStatusDelivered:
                    delivered = YES;
                    break;

                case LYRRecipientStatusRead:
                    readCount += 1;
                    break;
            }
        }];

        if (readCount) {
            NSString *participantString = readCount > 1 ? @"Participants" : @"Participant";
            statusString = [NSString stringWithFormat:@"Read by %lu %@", (unsigned long)readCount, participantString];

        } else if (pending) {
            statusString = @"Pending";

        } else if (delivered) {
            statusString = @"Delivered";

        } else if (sent) {
            statusString = @"Sent";
        }

    } else {
        __block NSString *blockStatusString = [NSString new];

        [mutableRecipientStatus enumerateKeysAndObjectsUsingBlock: ^(NSString *userID, NSNumber *statusNumber, BOOL *stop) {
            if ([userID isEqualToString: APP_DELEGATE.layerClient.authenticatedUser.userID]) return;

            LYRRecipientStatus status = statusNumber.integerValue;
            switch (status) {
                case LYRRecipientStatusInvalid:
                    blockStatusString = @"Not Sent";
                    break;

                case LYRRecipientStatusPending:
                    blockStatusString = @"Pending";
                    break;

                case LYRRecipientStatusSent:
                    blockStatusString = @"Sent";
                    break;

                case LYRRecipientStatusDelivered:
                    blockStatusString = @"Delivered";
                    break;

                case LYRRecipientStatusRead:
                    blockStatusString = @"Read";
                    break;
            }
        }];

        statusString = blockStatusString;
    }

    return [[NSAttributedString alloc] initWithString: statusString attributes: @{ NSFontAttributeName: [UIFont boldSystemFontOfSize: 11] }];
}

#pragma mark - ATLAddressBarControllerDelegate

/**
 Atlas - Informs the delegate that the user tapped the `addContacts` icon in the
 `ATLAddressBarViewController`. Atlas Messenger presents an `ATLParticipantPickerController`.
 */
- (void)
addressBarViewController: (ATLAddressBarViewController *) addressBarViewController
didTapAddContactsButton:  (UIButton *)                    addContactsButton
{
    NSSet *selectedParticipantIDs = [addressBarViewController.selectedParticipants valueForKey: @"userID"];

    if (!selectedParticipantIDs) {
        selectedParticipantIDs = [NSSet new];
    }

    LYRQuery *query = [LYRQuery queryWithQueryableClass: LYRIdentity.class];

    query.predicate = [LYRPredicate predicateWithProperty: @"userID"
                                    predicateOperator:     LYRPredicateOperatorIsNotIn
                                    value:                 selectedParticipantIDs     ];

    NSError *error;
    NSOrderedSet *identities = [self.layerClient executeQuery: query error: &error];
    if (error) {
        AlertWithError(error);
        return;
    }

    ParticipantTableViewController *controller = [ParticipantTableViewController participantTableViewControllerWithParticipants: identities.set
                                                                                 sortType:                                       ATLParticipantPickerSortTypeFirstName];

    controller.blockedParticipantIdentifiers = [self.layerClient.policies valueForKey: @"sentByUserID"];
    controller.delegate = self;
    controller.allowsMultipleSelection = NO;

    UINavigationController *navigationController = [[UINavigationController alloc] initWithRootViewController: controller];
    [self.navigationController presentViewController: navigationController animated: YES completion: nil];
}

/**
 Atlas - Informs the delegate that the user is searching for participants. Atlas
 Messengers queries for participants whose `fullName` property contains the given
 search string.
 */
- (void)
addressBarViewController:          (ATLAddressBarViewController *)   addressBarViewController
searchForParticipantsMatchingText: (NSString *)                      searchText
completion:                        (void (^)(NSArray *participants)) completion
{
    LYRQuery *query = [LYRQuery queryWithQueryableClass: LYRIdentity.class];

    query.predicate = [LYRPredicate predicateWithProperty: @"displayName"
                                    predicateOperator:     LYRPredicateOperatorLike
                                    value:                 [searchText stringByAppendingString: @"%"]];

    [self.layerClient executeQuery: query completion: ^(NSOrderedSet<id<ATLParticipant>> * _Nullable resultSet, NSError * _Nullable error) {
        if (resultSet) {
            completion(resultSet.array);

        } else {
            completion([NSArray array]);
        }
    }];
}

/**
 Atlas - Informs the delegate that the user tapped on the `ATLAddressBarViewController`
 while it was disabled. Atlas Messenger presents an `ATLConversationDetailViewController`
 in response.
 */
- (void) addressBarViewControllerDidSelectWhileDisabled: (ATLAddressBarViewController *) addressBarViewController
{
    [self detailsButtonTapped];
}

#pragma mark - ATLParticipantTableViewControllerDelegate

/**
 Atlas - Informs the delegate that the user selected an participant. Atlas Messenger in
 turn, informs the `ATLAddressBarViewController` of the selection.
 */
- (void)
participantTableViewController: (ATLParticipantTableViewController *) participantTableViewController
didSelectParticipant:           (id <ATLParticipant>)                 participant
{
    [self.addressBarController selectParticipant: participant];
    [self.navigationController dismissViewControllerAnimated: YES completion: nil];
}

/**
 Atlas - Informs the delegate that the user is searching for participants. Atlas
 Messengers queries for participants whose `fullName` property contains the give
 search string.
 */
- (void)
participantTableViewController: (ATLParticipantTableViewController *) participantTableViewController
didSearchWithString:            (NSString *)                          searchText
completion:                     (void (^)(NSSet *))                   completion
{
    LYRQuery *query = [LYRQuery queryWithQueryableClass: LYRIdentity.class];

    query.predicate = [LYRPredicate predicateWithProperty: @"displayName"
                                    predicateOperator:     LYRPredicateOperatorLike
                                    value:                 searchText              ];

    [self.layerClient executeQuery: query completion: ^(NSOrderedSet<id <ATLParticipant>> * _Nullable resultSet, NSError * _Nullable error) {
        if (resultSet) {
            completion(resultSet.set);

        } else {
            completion([NSSet set]);
        }
    }];
}

#pragma mark - LSConversationDetailViewControllerDelegate

/**
 Atlas - Informs the delegate that the user has tapped the `Share My Current Location`
 button. Atlas Messenger sends a message into the current conversation with the current
 location.
 */
- (void) conversationDetailViewControllerDidSelectShareLocation: (ConversationDetailViewController *) conversationDetailViewController
{
    [self sendLocationMessage];
    [self.navigationController popViewControllerAnimated: YES];
}

/**
 Atlas - Informs the delegate that the conversation has changed. Atlas Messenger updates its conversation and the current view controller's title in response.
 */
- (void)
conversationDetailViewController: (ConversationDetailViewController *) conversationDetailViewController
didChangeConversation:            (LYRConversation *)                  conversation
{
    self.conversation = conversation;
    [self configureTitle];
}

#pragma mark - Details Button Actions

- (void) addDetailsButton
{
    if (self.navigationItem.rightBarButtonItem) return;

    UIBarButtonItem *detailsButtonItem = [[UIBarButtonItem alloc] initWithTitle: DetailsButtonLabel
                                                                  style:         UIBarButtonItemStylePlain
                                                                  target:        self
                                                                  action:        @selector(detailsButtonTapped)];

    detailsButtonItem.accessibilityLabel = DetailsButtonAccessibilityLabel;
    self.navigationItem.rightBarButtonItem = detailsButtonItem;
}

- (void) detailsButtonTapped
{
    ConversationDetailViewController *detailViewController = [ConversationDetailViewController conversationDetailViewControllerWithConversation: self.conversation];

    detailViewController.detailDelegate = self;

    [self.navigationController pushViewController: detailViewController animated: YES];
}

#pragma mark - Notification Handlers

- (void) conversationMetadataDidChange: (NSNotification *) notification
{
    if (!self.conversation) return;
    if (!notification.object) return;
    if (![notification.object isEqual: self.conversation]) return;

    [self configureTitle];
}

#pragma mark - Helpers

- (void) configureTitle
{
    if ([self.conversation.metadata valueForKey: ConversationMetadataNameKey]) {
        NSString *conversationTitle = [self.conversation.metadata valueForKey: ConversationMetadataNameKey];

        if (conversationTitle.length) {
            self.title = conversationTitle;

        } else {
            self.title = [self defaultTitle];
        }

    } else {
        self.title = [self defaultTitle];
    }
}

- (NSString *) defaultTitle
{
    if (!self.conversation) return @"New Message";

    NSMutableSet *otherParticipants = [self.conversation.participants mutableCopy];
    NSPredicate *predicate = [NSPredicate predicateWithFormat: @"userID != %@", self.layerClient.authenticatedUser.userID];
    [otherParticipants filterUsingPredicate: predicate];

    if (otherParticipants.count == 0) {
        return @"Personal";

    } else if (otherParticipants.count == 1) {
        LYRIdentity *otherIdentity = [otherParticipants anyObject];
        id<ATLParticipant> participant = [self conversationViewController: self participantForIdentity: otherIdentity];
        return participant ? participant.firstName : @"Message";

    } else if (otherParticipants.count > 1) {
        NSUInteger participantCount = 0;
        id<ATLParticipant> knownParticipant;

        for (LYRIdentity *identity in otherParticipants) {
            id <ATLParticipant> participant = [self conversationViewController: self participantForIdentity: identity];

            if (participant) {
                participantCount += 1;
                knownParticipant = participant;
            }
        }

        if (participantCount == 1) {
            return knownParticipant.firstName;

        } else if (participantCount > 1) {
            return @"Group";
        }
    }

    return @"Message";
}

#pragma mark - Link Tap Handler

- (void) userDidTapLink: (NSNotification *) notification
{
    [[UIApplication sharedApplication] openURL: notification.object];
}

- (void) configureUserInterfaceAttributes
{
    [[ATLIncomingMessageCollectionViewCell appearance] setBubbleViewColor: ATLLightGrayColor()];
    [[ATLIncomingMessageCollectionViewCell appearance] setMessageTextColor: [UIColor blackColor]];
    [[ATLIncomingMessageCollectionViewCell appearance] setMessageLinkTextColor: ATLBlueColor()];

    [[ATLOutgoingMessageCollectionViewCell appearance] setBubbleViewColor: ATLBlueColor()];
    [[ATLOutgoingMessageCollectionViewCell appearance] setMessageTextColor: [UIColor whiteColor]];
    [[ATLOutgoingMessageCollectionViewCell appearance] setMessageLinkTextColor: [UIColor whiteColor]];
}

- (void) registerNotificationObservers
{
    [[NSNotificationCenter defaultCenter] addObserver: self
                                          selector:    @selector(userDidTapLink:)
                                          name:        ATLUserDidTapLinkNotification
                                          object:      nil                          ];

    [[NSNotificationCenter defaultCenter] addObserver: self
                                          selector:    @selector(conversationMetadataDidChange:)
                                          name:        ConversationMetadataDidChangeNotification
                                          object:      nil                                      ];

    [[NSNotificationCenter defaultCenter] addObserver: self
                                          selector:    @selector(deviceOrientationDidChange:)
                                          name:        UIDeviceOrientationDidChangeNotification
                                          object:      nil                                     ];
}

@end
