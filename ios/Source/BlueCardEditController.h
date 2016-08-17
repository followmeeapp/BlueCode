//
//  BlueCardEditController.h
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface BlueCardEditController : UITableViewController

@property (nonatomic, weak) IBOutlet UIImageView *avatarImageView;
@property (nonatomic, weak) IBOutlet UIImageView *coverPhotoImageView;

@property (nonatomic, weak) IBOutlet UITextField *fullNameTextField;
@property (nonatomic, weak) IBOutlet UITextField *locationTextField;
@property (nonatomic, weak) IBOutlet UITextView *bioTextView;

@property (nonatomic, weak) IBOutlet UIButton *bioButton;

@end
