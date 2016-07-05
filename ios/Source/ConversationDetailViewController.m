//
//  ConversationDetailViewController.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "ConversationDetailViewController.h"

#import "Follow.h"
#import "ParticipantTableViewController.h"
#import "CenterTextTableViewCell.h"
#import "InputTableViewCell.h"

#import <Atlas/Atlas.h>
#import <CoreLocation/CoreLocation.h>
#import <SVProgressHUD/SVProgressHUD.h>

typedef NS_ENUM(NSInteger, ConversationDetailTableSection) {
    ConversationDetailTableSectionMetadata,
    ConversationDetailTableSectionParticipants,
    ConversationDetailTableSectionLocation,
    ConversationDetailTableSectionLeave,
    ConversationDetailTableSectionCount,
};

typedef NS_ENUM(NSInteger, ActionSheetTag) {
    ActionSheetBlockUser,
    ActionSheetLeaveConversation,
};

NSString *const ConversationDetailViewControllerTitle = @"Details";
NSString *const ConversationDetailTableViewAccessibilityLabel = @"Conversation Detail Table View";
NSString *const AddParticipantsAccessibilityLabel = @"Add Participants";
NSString *const ConversationNamePlaceholderText = @"Enter Conversation Name";
NSString *const ConversationMetadataNameKey = @"conversationName";

NSString *const ShareLocationText = @"Send My Current Location";
NSString *const DeleteConversationText = @"Delete Conversation";
NSString *const LeaveConversationText = @"Leave Conversation";

static NSString *const ParticipantCellIdentifier = @"ParticipantCellIdentifier";
static NSString *const DefaultCellIdentifier = @"DefaultCellIdentifier";
static NSString *const InputCellIdentifier = @"InputCell";
static NSString *const CenterContentCellIdentifier = @"CenterContentCellIdentifier";

static NSString *const PlusIconName = @"AtlasResource.bundle/plus";
static NSString *const BlockIconName = @"AtlasResource.bundle/block";

@interface ConversationDetailViewController () <ATLParticipantTableViewControllerDelegate, UITextFieldDelegate, UIActionSheetDelegate>

@property (nonatomic) LYRConversation *conversation;
@property (nonatomic) NSMutableArray *participants;
@property (nonatomic) NSIndexPath *indexPathToRemove;
@property (nonatomic) CLLocationManager *locationManager;

@end

@implementation ConversationDetailViewController

+ (instancetype) conversationDetailViewControllerWithConversation: (LYRConversation *) conversation
{
    return [[self alloc] initWithConversation: conversation];
}

- (instancetype) initWithConversation: (LYRConversation *) conversation
{
    if (self = [super initWithStyle: UITableViewStyleGrouped]) {
        _conversation = conversation;
    }

    return self;
}

- (void) viewDidLoad
{
    [super viewDidLoad];

    self.title = ConversationDetailViewControllerTitle;
    self.tableView.sectionHeaderHeight = 48.0f;
    self.tableView.sectionFooterHeight = 0.0f;
    self.tableView.rowHeight = 48.0f;
    self.tableView.accessibilityLabel = ConversationDetailTableViewAccessibilityLabel;

    [self.tableView registerClass: CenterTextTableViewCell.class     forCellReuseIdentifier: CenterContentCellIdentifier];
    [self.tableView registerClass: ATLParticipantTableViewCell.class forCellReuseIdentifier: ParticipantCellIdentifier];
    [self.tableView registerClass: InputTableViewCell.class          forCellReuseIdentifier: InputCellIdentifier];
    [self.tableView registerClass: UITableViewCell.class             forCellReuseIdentifier: DefaultCellIdentifier];

    self.participants = [self filteredParticipants];

    [self configureAppearance];
    [self registerNotificationObservers];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - UITableViewDataSource

- (NSInteger) numberOfSectionsInTableView: (UITableView *) tableView
{
    return ConversationDetailTableSectionCount;
}

- (NSInteger)
tableView:             (UITableView *) tableView
numberOfRowsInSection: (NSInteger)     section
{
    switch (section) {
        case ConversationDetailTableSectionMetadata:
            return 1;

        case ConversationDetailTableSectionParticipants:
            return self.participants.count + 1; // Add a row for the `Add Participant` cell.

        case ConversationDetailTableSectionLocation:
            return 1;

        case ConversationDetailTableSectionLeave:
            return 1;

        default:
            return 0;
    }
}

- (UITableViewCell *)
tableView:             (UITableView *) tableView
cellForRowAtIndexPath: (NSIndexPath *) indexPath
{
    switch (indexPath.section) {
        case ConversationDetailTableSectionMetadata: {
            InputTableViewCell *cell = [self.tableView dequeueReusableCellWithIdentifier: InputCellIdentifier forIndexPath: indexPath];
            [self configureConversationNameCell: cell];
            return cell;
        }

        case ConversationDetailTableSectionParticipants:
            if (indexPath.row < self.participants.count) {
                ATLParticipantTableViewCell *cell = [self.tableView dequeueReusableCellWithIdentifier: ParticipantCellIdentifier forIndexPath: indexPath];
                [self configureParticipantCell: cell atIndexPath: indexPath];
                return cell;

            } else {
                UITableViewCell *cell = [self.tableView dequeueReusableCellWithIdentifier: DefaultCellIdentifier forIndexPath: indexPath];
                cell.textLabel.attributedText = [self addParticipantAttributedString];
                cell.accessibilityLabel = AddParticipantsAccessibilityLabel;
                cell.imageView.image = [UIImage imageNamed: PlusIconName];
                return cell;
            }

        case ConversationDetailTableSectionLocation: {
            UITableViewCell *cell = [self.tableView dequeueReusableCellWithIdentifier: DefaultCellIdentifier forIndexPath: indexPath];
            cell.textLabel.text = ShareLocationText;
            cell.textLabel.textColor = ATLBlueColor();
            cell.textLabel.font = [UIFont systemFontOfSize: 17];
            return cell;
        }

        case ConversationDetailTableSectionLeave: {
            CenterTextTableViewCell *cell = [self.tableView dequeueReusableCellWithIdentifier:CenterContentCellIdentifier];
            cell.centerTextLabel.textColor = ATLRedColor();
            cell.centerTextLabel.text = self.conversation.participants.count > 2 ? LeaveConversationText : DeleteConversationText;
            return cell;
        }

        default:
            return nil;
    }
}

- (NSString *)
tableView:               (UITableView *) tableView
titleForHeaderInSection: (NSInteger)     section
{
    switch ((ConversationDetailTableSection)section) {
        case ConversationDetailTableSectionMetadata:
            return @"Conversation Name";

        case ConversationDetailTableSectionParticipants:
            return @"Participants";

        case ConversationDetailTableSectionLocation:
            return @"Location";

        default:
            return nil;
    }
}

- (BOOL)
tableView:             (UITableView *) tableView
canEditRowAtIndexPath: (NSIndexPath *) indexPath
{
    if (indexPath.section == ConversationDetailTableSectionParticipants) {
        // Prevent removal in 1 to 1 conversations.
        if (self.conversation.participants.count < 3) return NO;

        BOOL canEdit = indexPath.row < self.participants.count;
        return canEdit;
    }

    return NO;
}

- (void)
tableView:          (UITableView *)               tableView
commitEditingStyle: (UITableViewCellEditingStyle) editingStyle
forRowAtIndexPath:  (NSIndexPath *)               indexPath
{
    self.indexPathToRemove = indexPath;
    NSString *blockString = [self blockedParticipantAtIndexPath: indexPath] ? @"Unblock" : @"Block";

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    UIActionSheet *actionSheet = [[UIActionSheet alloc] initWithTitle:          nil
                                                        delegate:               self
                                                        cancelButtonTitle:      @"Cancel"
                                                        destructiveButtonTitle: @"Remove"
                                                        otherButtonTitles:      blockString, nil];

#pragma clang diagnostic pop

    actionSheet.tag = ActionSheetBlockUser;

    [actionSheet showInView: self.view];
}


#pragma mark - UITableViewDelegate

- (void)
tableView:(UITableView *)tableView
didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    switch ((ConversationDetailTableSection)indexPath.section) {
        case ConversationDetailTableSectionParticipants:
            if (indexPath.row == self.participants.count) {
                [self presentParticipantPicker];
            }
            break;

        case ConversationDetailTableSectionLocation:
            [self shareLocation];
            break;

        case ConversationDetailTableSectionLeave:
            [self confirmLeaveConversation];
            break;

        default:
            break;
    }
}

- (NSArray *)
tableView:                    (UITableView *) tableView
editActionsForRowAtIndexPath: (NSIndexPath *) indexPath
{
    UITableViewRowAction *removeAction = [UITableViewRowAction rowActionWithStyle: UITableViewRowActionStyleDefault
                                                               title:              @"Remove"
                                                               handler:            ^(UITableViewRowAction *action, NSIndexPath *indexPath) {
        [self removeParticipantAtIndexPath: indexPath];
    }];

    removeAction.backgroundColor = ATLGrayColor();

    NSString *blockString = [self blockedParticipantAtIndexPath: indexPath] ? @"Unblock" : @"Block";

    UITableViewRowAction *blockAction = [UITableViewRowAction rowActionWithStyle: UITableViewRowActionStyleDefault
                                                              title:              blockString
                                                              handler:            ^(UITableViewRowAction *action, NSIndexPath *indexPath) {
        [self blockParticipantAtIndexPath: indexPath];
    }];

    blockAction.backgroundColor = ATLRedColor();

    return @[removeAction, blockAction];
}

#pragma mark - Cell Configuration

- (void) configureConversationNameCell: (InputTableViewCell *) cell
{
    cell.selectionStyle = UITableViewCellSelectionStyleNone;
    cell.textField.delegate = self;
    cell.textField.clearButtonMode = UITextFieldViewModeWhileEditing;
    cell.guideText = @"Name:";
    cell.placeHolderText = @"Enter Conversation Name";
    NSString *conversationName = [self.conversation.metadata valueForKey:ConversationMetadataNameKey];
    cell.textField.text = conversationName;
}

- (void)
configureParticipantCell:(ATLParticipantTableViewCell *)cell
atIndexPath:(NSIndexPath *)indexPath
{
    id <ATLParticipant> participant = [self.participants objectAtIndex: indexPath.row];

    if ([self blockedParticipantAtIndexPath: indexPath]) {
        cell.accessoryView.accessibilityLabel = @"Blocked";
        cell.accessoryView = [[UIImageView alloc] initWithImage: [UIImage imageNamed: BlockIconName]];
    }

    [cell presentParticipant: participant withSortType: ATLParticipantPickerSortTypeFirstName shouldShowAvatarItem: YES];
    cell.selectionStyle = UITableViewCellSelectionStyleNone;
}

- (NSAttributedString *) addParticipantAttributedString
{
    NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString: @"Add Participant"];

    NSRange range = NSMakeRange(0, attributedString.length);
    [attributedString addAttribute: NSForegroundColorAttributeName value: ATLBlueColor() range: range];
    [attributedString addAttribute: NSFontAttributeName value: [UIFont systemFontOfSize:17]  range: range];

    return attributedString;
}

#pragma mark - UIActionSheetDelegate

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
- (void)
actionSheet:          (UIActionSheet *) actionSheet
clickedButtonAtIndex: (NSInteger)       buttonIndex
{
    if (actionSheet.tag == ActionSheetBlockUser) {
        if (buttonIndex == actionSheet.destructiveButtonIndex) {
            [self removeParticipantAtIndexPath: self.indexPathToRemove];

        } else if (buttonIndex == actionSheet.firstOtherButtonIndex) {
            [self blockParticipantAtIndexPath: self.indexPathToRemove];

        } else if (buttonIndex == actionSheet.cancelButtonIndex) {
            [self setEditing: NO animated: YES];
        }

        self.indexPathToRemove = nil;

    } else if (actionSheet.tag == ActionSheetLeaveConversation) {
        if (buttonIndex == actionSheet.destructiveButtonIndex) {
            self.conversation.participants.count > 2 ? [self leaveConversation] : [self deleteConversation];
        }
    }
}
#pragma clang diagnostic pop

#pragma mark - Actions

- (void) presentParticipantPicker
{
    LYRQuery *query = [LYRQuery queryWithQueryableClass: LYRIdentity.class];

    query.predicate = [LYRPredicate predicateWithProperty: @"userID"
                                    predicateOperator:     LYRPredicateOperatorIsNotIn
                                    value:                 [self.conversation.participants valueForKey: @"userID"]];

    NSError *error = nil;
    NSOrderedSet *identities = [APP_DELEGATE.layerClient executeQuery: query error: &error];

    if (error) {
        AlertWithError(error);
        return;
    }

    ParticipantTableViewController  *controller = [ParticipantTableViewController participantTableViewControllerWithParticipants: identities.set
                                                                                  sortType:                                       ATLParticipantPickerSortTypeFirstName];
    controller.delegate = self;
    controller.allowsMultipleSelection = NO;

    UINavigationController *navigationController = [[UINavigationController alloc] initWithRootViewController: controller];
    [self.navigationController presentViewController: navigationController animated: YES completion: nil];
}

- (void) removeParticipantAtIndexPath: (NSIndexPath *) indexPath
{
    id <ATLParticipant> participant = self.participants[indexPath.row];

    NSError *error;
    BOOL success = [self.conversation removeParticipants: [NSSet setWithObject: [participant userID]] error: &error];

    if (!success) {
        AlertWithError(error);
        return;
    }

    [self.participants removeObjectAtIndex: indexPath.row];
    [self.tableView deleteRowsAtIndexPaths: @[indexPath] withRowAnimation: UITableViewRowAnimationLeft];
}

- (void) blockParticipantAtIndexPath:(NSIndexPath *)indexPath
{
    id <ATLParticipant> participant = [self.participants objectAtIndex: indexPath.row];
    LYRPolicy *policy =  [self blockedParticipantAtIndexPath: indexPath];

    if (policy) {
        NSError *error;
        [APP_DELEGATE.layerClient removePolicies: [NSSet setWithObject: policy] error: &error];

        if (error) {
            AlertWithError(error);
            return;
        }

    } else {
        [self blockParticipantWithIdentifier: [participant userID]];
    }

    [self.tableView reloadRowsAtIndexPaths: @[indexPath] withRowAnimation: UITableViewRowAnimationAutomatic];
}

- (void) blockParticipantWithIdentifier: (NSString *) identitifer
{
    LYRPolicy *blockPolicy = [LYRPolicy policyWithType: LYRPolicyTypeBlock];
    blockPolicy.sentByUserID = identitifer;

    NSError *error;
    [APP_DELEGATE.layerClient addPolicies: [NSSet setWithObject: blockPolicy] error: &error];

    if (error) {
        AlertWithError(error);
        return;
    }

    [SVProgressHUD showSuccessWithStatus: @"Participant Blocked"];
}

- (void) shareLocation
{
    [self.detailDelegate conversationDetailViewControllerDidSelectShareLocation: self];
}

- (void) confirmLeaveConversation
{
    NSString *destructiveButtonTitle = self.conversation.participants.count > 2 ? LeaveConversationText : DeleteConversationText;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    UIActionSheet *actionSheet = [[UIActionSheet alloc] initWithTitle:          nil
                                                        delegate:               self
                                                        cancelButtonTitle:      @"Cancel"
                                                        destructiveButtonTitle: destructiveButtonTitle
                                                        otherButtonTitles:      nil                   ];

#pragma clang diagnostic pop

    actionSheet.tag = ActionSheetLeaveConversation;

    [actionSheet showInView: self.view];
}

- (void) leaveConversation
{
    NSSet *participants = [NSSet setWithObject: APP_DELEGATE.layerClient.authenticatedUser.userID];

    NSError *error = nil;
    [self.conversation removeParticipants: participants error: &error];

    if (error) {
        AlertWithError(error);
        return;
    }

    [self.navigationController popToRootViewControllerAnimated: YES];
}

- (void) deleteConversation
{
    NSError *error = nil;
    [self.conversation delete: LYRDeletionModeAllParticipants error: &error];

    if (error) {
        AlertWithError(error);
        return;
    }

    [self.navigationController popToRootViewControllerAnimated: YES];
}

#pragma mark - ATLParticipantTableViewControllerDelegate

- (void)
participantTableViewController: (ATLParticipantTableViewController *) participantTableViewController
didSelectParticipant:           (id <ATLParticipant>)                 participant
{
    [self.navigationController dismissViewControllerAnimated: YES completion: nil];

    [self.participants addObject: participant];

    if (self.conversation.participants.count < 3) {
        [self switchToConversationForParticipants];

    } else {
        NSError *error;
        BOOL success = [self.conversation addParticipants: [NSSet setWithObject: participant.userID] error: &error];
        if (!success) {
            AlertWithError(error);
            return;
        }
    }

    [self.tableView reloadData];
}

- (void)
participantTableViewController: (ATLParticipantTableViewController *) participantTableViewController
didSearchWithString:            (NSString *)                          searchText
completion:                     (void (^)(NSSet *))                   completion
{
    LYRQuery *query = [LYRQuery queryWithQueryableClass: LYRIdentity.class];

    query.predicate = [LYRPredicate predicateWithProperty: @"displayName" predicateOperator: LYRPredicateOperatorLike value: searchText];

    [APP_DELEGATE.layerClient executeQuery: query completion: ^(NSOrderedSet<id<LYRQueryable>> * _Nullable resultSet, NSError * _Nullable error) {
        if (resultSet) {
            completion(resultSet.set);

        } else {
            completion([NSSet set]);
        }
    }];
}

#pragma mark - Conversation Configuration

- (void) switchToConversationForParticipants
{
    NSSet *participantIdentifiers = [self.participants valueForKey: @"userID"];
    LYRConversation *conversation = [APP_DELEGATE.layerClient existingConversationForParticipants: participantIdentifiers];

    if (!conversation) {
        conversation = [APP_DELEGATE.layerClient newConversationWithParticipants: participantIdentifiers options: nil error: nil];
    }

    [self.detailDelegate conversationDetailViewController: self didChangeConversation: conversation];
    self.conversation = conversation;
}

- (LYRPolicy *) blockedParticipantAtIndexPath: (NSIndexPath *) indexPath
{
    NSOrderedSet *policies = APP_DELEGATE.layerClient.policies;
    id <ATLParticipant> participant = self.participants[indexPath.row];

    NSPredicate *policyPredicate = [NSPredicate predicateWithFormat: @"SELF.sentByUserID = %@", [participant userID]];

    NSOrderedSet *filteredPolicies = [policies filteredOrderedSetUsingPredicate: policyPredicate];

    if (filteredPolicies.count) {
        return filteredPolicies.firstObject;

    } else {
        return nil;
    }
}

#pragma mark - UITextFieldDelegate

- (void) textFieldDidEndEditing: (UITextField *) textField
{
    NSString *title = [self.conversation.metadata valueForKey: ConversationMetadataNameKey];

    if (![textField.text isEqualToString: title]) {
        [self.conversation setValue: textField.text forMetadataAtKeyPath: ConversationMetadataNameKey];
    }
}

- (BOOL) textFieldShouldReturn: (UITextField *) textField
{
    if (textField.text.length > 0) {
        [self.conversation setValue: textField.text forMetadataAtKeyPath: ConversationMetadataNameKey];

    } else {
        [self.conversation deleteValueForMetadataAtKeyPath: ConversationMetadataNameKey];
    }

    [textField resignFirstResponder];

    return YES;
}

#pragma mark - Notification Handlers

- (void) conversationMetadataDidChange: (NSNotification *) notification
{
    if (!self.conversation) return;
    if (!notification.object) return;
    if (![notification.object isEqual: self.conversation]) return;

    NSIndexPath *nameIndexPath = [NSIndexPath indexPathForRow: 0 inSection: ConversationDetailTableSectionMetadata];
    InputTableViewCell *nameCell = (InputTableViewCell *)[self.tableView cellForRowAtIndexPath: nameIndexPath];

    if (!nameCell) return;
    if ([nameCell.textField isFirstResponder]) return;

    [self configureConversationNameCell: nameCell];
}

- (void) conversationParticipantsDidChange: (NSNotification *) notification
{
    if (!self.conversation) return;
    if (!notification.object) return;
    if (![notification.object isEqual: self.conversation]) return;

    [self.tableView beginUpdates];

    NSSet *existingParticipants = [NSSet setWithArray: self.participants];

    NSMutableArray *deletedIndexPaths = [NSMutableArray new];
    NSMutableIndexSet *deletedIndexSet = [NSMutableIndexSet new];
    NSMutableSet *deletedParticipants = [existingParticipants mutableCopy];
    [deletedParticipants minusSet: self.conversation.participants];

    for (LYRIdentity *deletedIdentity in deletedParticipants) {
        NSUInteger row = [self.participants indexOfObject: deletedIdentity];
        [deletedIndexSet addIndex: row];
        NSIndexPath *indexPath = [NSIndexPath indexPathForRow: row inSection: ConversationDetailTableSectionParticipants];
        [deletedIndexPaths addObject: indexPath];
    }

    [self.participants removeObjectsAtIndexes: deletedIndexSet];
    [self.tableView deleteRowsAtIndexPaths: deletedIndexPaths withRowAnimation: UITableViewRowAnimationAutomatic];

    NSMutableArray *insertedIndexPaths = [NSMutableArray new];
    NSMutableSet *insertedParticipants = [self.conversation.participants mutableCopy];
    [insertedParticipants removeObject: APP_DELEGATE.layerClient.authenticatedUser];
    [insertedParticipants minusSet: existingParticipants];

    for (LYRIdentity *identity in insertedParticipants) {
        [self.participants addObject: identity];
        NSIndexPath *indexPath = [NSIndexPath indexPathForRow: self.participants.count - 1 inSection: ConversationDetailTableSectionParticipants];
        [insertedIndexPaths addObject: indexPath];
    }

    [self.tableView insertRowsAtIndexPaths: insertedIndexPaths withRowAnimation: UITableViewRowAnimationAutomatic];
    [self.tableView endUpdates];
}

#pragma mark - Helpers

- (NSMutableArray *) filteredParticipants
{
    NSMutableArray *participants = [[self.conversation.participants allObjects] mutableCopy];
    [participants removeObject: APP_DELEGATE.layerClient.authenticatedUser];
    return participants;
}

- (void) configureAppearance
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    [[ATLParticipantTableViewCell appearanceWhenContainedIn: [self class], nil] setTitleColor: [UIColor blackColor]];
    [[ATLParticipantTableViewCell appearanceWhenContainedIn: [self class], nil] setTitleFont: [UIFont systemFontOfSize: 17]];
    [[ATLParticipantTableViewCell appearanceWhenContainedIn: [self class], nil] setBoldTitleFont: [UIFont systemFontOfSize: 17]];

    [[ATLAvatarImageView appearanceWhenContainedIn: [ATLParticipantTableViewCell class], nil] setAvatarImageViewDiameter: 32];
#pragma clang diagnostic pop
}

- (void) registerNotificationObservers
{
    [[NSNotificationCenter defaultCenter] addObserver: self
                                          selector:    @selector(conversationMetadataDidChange:)
                                          name:        ConversationMetadataDidChangeNotification
                                          object:      nil                                      ];

    [[NSNotificationCenter defaultCenter] addObserver: self
                                          selector:    @selector(conversationParticipantsDidChange:)
                                          name:        ConversationParticipantsDidChangeNotification
                                          object:      nil                                          ];
}

@end
