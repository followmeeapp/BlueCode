//
//  TileCollectionViewCell.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

@interface TileCollectionViewCell : UICollectionViewCell

@property (nonatomic, weak) IBOutlet UIView *buySizeView;
@property (nonatomic, weak) IBOutlet UIView *dividerView;

@property (nonatomic, weak) IBOutlet UIImageView *imageView;

@property (nonatomic, weak) IBOutlet UILabel *nameLabel;
@property (nonatomic, weak) IBOutlet UILabel *taglineLabel;

@property (nonatomic, weak) IBOutlet UIView *priceView;
@property (nonatomic, weak) IBOutlet UIButton *priceButton;
@property (nonatomic, weak) IBOutlet UILabel *priceLabel;

@property (nonatomic, weak) IBOutlet UIButton *sizeButton;
@property (nonatomic, weak) IBOutlet UIButton *buyButton;

@property (nonatomic, weak) IBOutlet UILabel *sizeLabel;
@property (nonatomic, weak) IBOutlet UILabel *chevronLabel;
@property (nonatomic, weak) IBOutlet UILabel *buyLabel;

- (void) showPriceButton;
- (void) showBuyButtonWithSize: (NSString *) size;

@end
