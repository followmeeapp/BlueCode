//
//  NotificationCell.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

@interface NotificationCell : UITableViewCell

@property (strong, nonatomic) IBOutlet UILabel *senderName;
@property (strong, nonatomic) IBOutlet UILabel *dateLabel;
@property (strong, nonatomic) IBOutlet UILabel *messageLabel;
@property (strong, nonatomic) IBOutlet UILabel *indicatorLabel;

- (void) updateSenderName: (NSString *) senderName;
- (void) updateDate: (NSString *) date;
- (void) updateMessageLabel: (NSString *) message;

@end
