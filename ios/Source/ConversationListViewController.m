//
//  ConversationListViewController.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "ConversationListViewController.h"

#import "Follow.h"
#import "ConversationViewController.h"
#import "ConversationDetailViewController.h"
#import "NavigationController.h"

@interface ConversationListViewController () <ATLConversationListViewControllerDelegate, ATLConversationListViewControllerDataSource, UIActionSheetDelegate>

@end

@implementation ConversationListViewController

- (void) viewDidLoad
{
    [super viewDidLoad];

    self.delegate = self;
    self.dataSource = self;
    self.allowsEditing = YES;

    [self registerNotificationObservers];
}

- (void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

#pragma mark - ATLConversationListViewControllerDelegate

/**
 Atlas - Informs the delegate of a conversation selection. Atlas Messenger pushses a subclass of the `ATLConversationViewController`.
 */
- (void)
conversationListViewController: (ATLConversationListViewController *) conversationListViewController
didSelectConversation:          (LYRConversation *)                   conversation
{
    [self presentControllerWithConversation: conversation];
}

/**
 Atlas - Informs the delegate a conversation was deleted. Atlas Messenger does not need to react as the superclass will handle removing the conversation in response to a deletion.
 */
- (void)
conversationListViewController: (ATLConversationListViewController *) conversationListViewController
didDeleteConversation:          (LYRConversation *)                   conversation
deletionMode:                   (LYRDeletionMode)                     deletionMode
{
    NSLog(@"Conversation Successfully Deleted");
}

/**
 Atlas - Informs the delegate that a conversation deletion attempt failed. Atlas Messenger does not do anything in response.
 */
- (void)
conversationListViewController: (ATLConversationListViewController *) conversationListViewController
didFailDeletingConversation:    (LYRConversation *)                   conversation
deletionMode:                   (LYRDeletionMode)                     deletionMode
error:                          (NSError *)                           error
{
    NSLog(@"Conversation Deletion Failed with Error: %@", error);
}

/**
 Atlas - Informs the delegate that a search has been performed. Atlas messenger queries for, and returns objects conforming to the `ATLParticipant` protocol whose `fullName` property contains the search text.
 */
- (void)
conversationListViewController: (ATLConversationListViewController *)                    conversationListViewController
didSearchForText:               (nonnull NSString *)                                     searchText
completion:                     (nonnull void (^)(NSSet<id<ATLParticipant>> * _Nonnull)) completion
{
    LYRQuery *query = [LYRQuery queryWithQueryableClass: LYRIdentity.class];

    query.predicate = [LYRPredicate predicateWithProperty: @"displayName"
                                    predicateOperator:     LYRPredicateOperatorLike
                                    value:                 [searchText stringByAppendingString: @"%"]];

    [self.layerClient executeQuery: query completion: ^(NSOrderedSet<id<ATLParticipant>> * _Nullable resultSet, NSError * _Nullable error) {
        if (resultSet) {
            completion(resultSet.set);

        } else {
            completion([NSSet set]);
        }
    }];
}

- (id <ATLAvatarItem>)
conversationListViewController: (ATLConversationListViewController *) conversationListViewController
avatarItemForConversation:      (LYRConversation *)                   conversation
{
    NSMutableSet *participants = conversation.participants.mutableCopy;

    [participants removeObject: self.layerClient.authenticatedUser];

    return participants.anyObject;
}

#pragma mark - ATLConversationListViewControllerDataSource

/**
 Atlas - Returns a label that is used to represent the conversation. Atlas Messenger puts the name representing the `lastMessage.sentByUserID` property first in the string.
 */
- (NSString *)
conversationListViewController: (ATLConversationListViewController *) conversationListViewController
titleForConversation:           (LYRConversation *)                   conversation
{
    // If we have a Conversation name in metadata, return it.
    NSString *conversationTitle = conversation.metadata[ConversationMetadataNameKey];
    if (conversationTitle.length) return conversationTitle;

    NSMutableSet *participants = [conversation.participants mutableCopy];
    NSPredicate *predicate = [NSPredicate predicateWithFormat: @"userID != %@", self.layerClient.authenticatedUser.userID];
    [participants filterUsingPredicate: predicate];

    if (participants.count == 0) return @"Personal Conversation";
    if (participants.count == 1) return [[participants allObjects][0] displayName];

    NSMutableArray *firstNames = [NSMutableArray new];

    [participants enumerateObjectsUsingBlock: ^(id obj, BOOL *stop) {
        id <ATLParticipant> participant = obj;

        if (participant.firstName) {
            // Put the last message sender's name first
            if ([conversation.lastMessage.sender.userID isEqualToString: participant.userID]) {
                [firstNames insertObject: participant.firstName atIndex: 0];

            } else {
                [firstNames addObject: participant.firstName];
            }
        }
    }];

    NSString *firstNamesString = [firstNames componentsJoinedByString: @", "];
    return firstNamesString;
}

#pragma mark - Conversation Selection

// The following method handles presenting the correct `ConversationViewController`, regardeless of the current state of the navigation stack.
- (void) presentControllerWithConversation: (LYRConversation *) conversation
{
    ConversationViewController *existingConversationViewController = [self existingConversationViewController];

    if (existingConversationViewController && existingConversationViewController.conversation == conversation) {
        if (self.navigationController.topViewController == existingConversationViewController) return;

        [self.navigationController popToViewController: existingConversationViewController animated: YES];
        return;
    }

    BOOL shouldShowAddressBar = (conversation.participants.count > 2 || !conversation.participants.count);

    ConversationViewController *conversationViewController = [ConversationViewController conversationViewControllerWithLayerClient: APP_DELEGATE.layerClient];
    conversationViewController.displaysAddressBar = shouldShowAddressBar;
    conversationViewController.conversation = conversation;

    [self.navigationController presentViewController: conversationViewController animated: YES completion: nil];
}

#pragma mark - Actions

//- (void) settingsButtonTapped
//{
//    ATLMSettingsViewController *settingsViewController = [[ATLMSettingsViewController alloc] initWithStyle:UITableViewStyleGrouped];
//    settingsViewController.applicationController = self.applicationController;
//    settingsViewController.settingsDelegate = self;
//
//    UINavigationController *controller = [[UINavigationController alloc] initWithRootViewController:settingsViewController];
//    [self.navigationController presentViewController:controller animated:YES completion:nil];
//}

- (void) composeButtonTapped
{
    [self presentControllerWithConversation: nil];
}

#pragma mark - Conversation Selection From Push Notification

- (void) selectConversation: (LYRConversation *) conversation
{
    if (!conversation) return;

    [self presentControllerWithConversation: conversation];
}

#pragma mark - Notification Handlers

- (void) conversationDeleted: (NSNotification *) notification
{
    if (self.Follow_navigationController.isAnimating) {
        [self.Follow_navigationController notifyWhenCompletionEndsUsingBlock: ^{
            [self conversationDeleted: notification];
        }];
        return;
    }

    ConversationViewController *conversationViewController = [self existingConversationViewController];
    if (!conversationViewController) return;

    LYRConversation *deletedConversation = notification.object;
    if (![conversationViewController.conversation isEqual: deletedConversation]) return;

    conversationViewController = nil;
    [self.navigationController popToViewController: self animated: YES];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Conversation Deleted"
                                                  message:           @"The conversation has been deleted."
                                                  delegate:          nil
                                                  cancelButtonTitle: @"OK"
                                                  otherButtonTitles: nil                                 ];
#pragma clang diagnostic pop
    [alertView show];
}

- (void) conversationParticipantsDidChange: (NSNotification *) notification
{
    if (self.Follow_navigationController.isAnimating) {
        [self.Follow_navigationController notifyWhenCompletionEndsUsingBlock: ^{
            [self conversationParticipantsDidChange: notification];
        }];
        return;
    }

    NSString *authenticatedUserID = APP_DELEGATE.layerClient.authenticatedUser.userID;
    if (!authenticatedUserID) return;

    LYRConversation *conversation = notification.object;
    if ([[conversation.participants valueForKeyPath: @"userID"] containsObject: authenticatedUserID]) return;

    ConversationViewController *conversationViewController = [self existingConversationViewController];
    if (!conversationViewController) return;
    if (![conversationViewController.conversation isEqual: conversation]) return;

    [self.navigationController popToViewController: self animated: YES];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Removed From Conversation"
                                                  message:           @"You have been removed from the conversation."
                                                  delegate:          nil
                                                  cancelButtonTitle: @"OK"
                                                  otherButtonTitles: nil                                           ];
#pragma clang diagnostic pop
    [alertView show];
}

#pragma mark - Helpers

- (ConversationViewController *) existingConversationViewController
{
    if (!self.navigationController) return nil;

    NSUInteger listViewControllerIndex = [self.navigationController.viewControllers indexOfObject: self];
    if (listViewControllerIndex == NSNotFound) return nil;

    NSUInteger nextViewControllerIndex = listViewControllerIndex + 1;
    if (nextViewControllerIndex >= self.navigationController.viewControllers.count) return nil;

    id nextViewController = [self.navigationController.viewControllers objectAtIndex: nextViewControllerIndex];
    if (![nextViewController isKindOfClass: ConversationViewController.class]) return nil;

    return nextViewController;
}

- (void) registerNotificationObservers
{
    [[NSNotificationCenter defaultCenter] addObserver: self
                                          selector:    @selector(conversationDeleted:)
                                          name:        ConversationDeletedNotification
                                          object:      nil                            ];

    [[NSNotificationCenter defaultCenter] addObserver: self
                                          selector:    @selector(conversationParticipantsDidChange:)
                                          name:        ConversationParticipantsDidChangeNotification
                                          object:      nil                                          ];
}

@end
