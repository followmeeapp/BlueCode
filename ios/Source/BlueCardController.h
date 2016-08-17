//
//  BlueCardController.h
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

@class CardObject;

@interface BlueCardController : UIViewController <UICollectionViewDataSource, UICollectionViewDelegate>

@property (nonatomic, assign) BOOL isCardLink;

@property (nonatomic, strong) CardObject *card;

@property (nonatomic, strong) NSIndexPath *indexPath;

@property (nonatomic, weak) IBOutlet UIButton *editButton;
@property (nonatomic, weak) IBOutlet UIButton *fastFollowButton;

@property (nonatomic, weak) IBOutlet UIImageView *background;
@property (nonatomic, weak) IBOutlet UIImageView *avatar;

@property (nonatomic, weak) IBOutlet UILabel *fullName;
@property (nonatomic, weak) IBOutlet UILabel *location;

@property (nonatomic, weak) IBOutlet UITextView *bio;

@property (nonatomic, weak) IBOutlet UICollectionView *networks;

@end
