//
//  ChatMessageCell.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

@interface ChatMessageCell : UITableViewCell

@property (nonatomic, weak) IBOutlet UILabel *deviceLabel;
@property (nonatomic, weak) IBOutlet UILabel *messageLabel;
@property (nonatomic, weak) IBOutlet UIImageView *messageStatus;

- (void) updateWithImage: (UIImage *) image;
- (void) removeImage;
- (void) assignText: (NSString *) text;

@end
