//
//  FollowCell.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

@interface FollowCell : UICollectionViewCell

@property (nonatomic, weak) IBOutlet UIView *cellView;

@property (nonatomic, weak) IBOutlet UIView *divider;

@property (nonatomic, weak) IBOutlet UIImageView *imageView;

@property (nonatomic, weak) IBOutlet UILabel *networkLabel;
@property (nonatomic, weak) IBOutlet UILabel *followeeLabel;

@property (nonatomic, weak) IBOutlet UISwitch *toggle;

@end
