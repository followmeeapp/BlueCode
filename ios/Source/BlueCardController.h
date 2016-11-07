//
//  BlueCardController.h
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

@class CardObject;

@interface BlueCardController : UIViewController <UICollectionViewDataSource, UICollectionViewDelegate>

@property (nonatomic, assign) BOOL isEditableCard;
@property (nonatomic, assign) BOOL shouldEditUserCard;

@property (nonatomic, assign) BOOL isCardLink;

@property (nonatomic, strong) CardObject *card;

@property (nonatomic, strong) NSIndexPath *indexPath;

@property (nonatomic, weak) IBOutlet UIImageView *avatar;
@property (nonatomic, weak) IBOutlet UIImageView *background;
@property (nonatomic, weak) IBOutlet UIImageView *opacityOverlay;

@property (nonatomic, weak) IBOutlet UIButton *avatarButton;
@property (nonatomic, weak) IBOutlet UIButton *backgroundButton;
@property (nonatomic, weak) IBOutlet UIButton *backgroundSettingsButton;

@property (nonatomic, weak) IBOutlet UITextField *fullName;
@property (nonatomic, weak) IBOutlet UITextField *location;

@property (nonatomic, weak) IBOutlet UITextView *bio;

@property (nonatomic, weak) IBOutlet UICollectionView *networks;

@property (nonatomic, weak) IBOutlet UIButton *shareButton;

@property (nonatomic, assign) BOOL isUserCard;

@end
