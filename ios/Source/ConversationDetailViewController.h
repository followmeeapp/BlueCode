//
//  ConversationDetailViewController.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import <LayerKit/LayerKit.h>

extern NSString *const ConversationDetailViewControllerTitle;
extern NSString *const ConversationMetadataNameKey;

@class ConversationDetailViewController;

/**
 @abstract The `ConversationDetailViewControllerDelegate` notifies its receiver of events that occur within the controller.
 */
@protocol ConversationDetailViewControllerDelegate <NSObject>

/**
 @abstract Informs the delegate that a user has elected to share the application's current location.
 @param conversationDetailViewController The `ATLMConversationDetailViewController` in which the selection occurred.
 */
- (void) conversationDetailViewControllerDidSelectShareLocation: (ConversationDetailViewController *) conversationDetailViewController;

/**
 @abstract Informs the delegate that a user has elected to switch the conversation.
 @param conversationDetailViewController The `ATLMConversationDetailViewController` in which the selection occurred.
 @param conversation The new `LYRConversation` object.
 @discussion The user changes the `LYRConversation` object in response to adding or deleting participants from a conversation.
 */
- (void)
conversationDetailViewController: (ConversationDetailViewController *) conversationDetailViewController
didChangeConversation:            (LYRConversation *)                  conversation;

@end

@interface ConversationDetailViewController : UITableViewController

@property (nonatomic) id <ConversationDetailViewControllerDelegate> detailDelegate;

+ (instancetype) conversationDetailViewControllerWithConversation: (LYRConversation *) conversation;

@end
