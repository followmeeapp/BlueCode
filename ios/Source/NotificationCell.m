//
//  NotificationCell.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "NotificationCell.h"

@implementation NotificationCell

- (void) updateSenderName: (NSString *) senderName
{
    self.senderName.text = senderName;
}

- (void) updateDate: (NSString *) date
{
    self.dateLabel.text = date;
}

- (void) updateMessageLabel: (NSString *) message
{
    self.messageLabel.text = message;
}

@end
