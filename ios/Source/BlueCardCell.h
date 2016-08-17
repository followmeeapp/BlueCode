//
//  BlueCardCell.h
//  Follow
//
//  Created by Erich Ocean on 7/30/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface BlueCardCell : UITableViewCell

@property (nonatomic, weak) IBOutlet UIImageView *avatar;
@property (nonatomic, weak) IBOutlet UIImageView *coverPhoto;

@property (nonatomic, weak) IBOutlet UILabel *fullName;
@property (nonatomic, weak) IBOutlet UILabel *location;

@end
