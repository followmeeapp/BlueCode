//
//  ConversationListViewController.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import <Atlas/Atlas.h>

@interface ConversationListViewController : ATLConversationListViewController

/**
 @abstract Programmatically simulates the selection of an `LYRConversation` object in the conversations table view.
 @discussion This method is used when opening the application in response to a push notification. When invoked, it
 will display the appropriate conversation on screen.
 */
- (void) selectConversation: (LYRConversation *) conversation;

@end
