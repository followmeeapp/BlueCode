//
//  TileCollectionViewCell.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "TileCollectionViewCell.h"

@interface TileCollectionViewCell ()
@end

@implementation TileCollectionViewCell

- (void) awakeFromNib
{
    [self addViewOutline: self.priceView];
    [self addViewOutline: self.buySizeView];
}

- (void) addViewOutline: (UIView *) view
{
    view.layer.cornerRadius = 12.0;
    view.layer.borderWidth = 0.5;
    view.layer.borderColor = [UIColor blackColor].CGColor;
}

- (void) showPriceButton
{
    self.priceView.layer.opacity = 1.0;
    self.priceLabel.layer.opacity = 1.0;
    self.priceButton.enabled = YES;

    self.sizeLabel.layer.opacity = 0.0;
    self.chevronLabel.layer.opacity = 0.0;
    self.buyLabel.layer.opacity = 0.0;
    self.buySizeView.layer.opacity = 0.0;
    self.dividerView.layer.opacity = 0.0;
    self.sizeButton.enabled = NO;
    self.buyButton.enabled = NO;
}

- (void) showBuyButtonWithSize: (NSString *) size
{
    self.priceView.layer.opacity = 0.0;
    self.priceLabel.layer.opacity = 0.0;
    self.priceButton.enabled = NO;

    self.sizeLabel.text = size ? size : @"?";
    self.sizeLabel.layer.opacity = 1.0;
    self.chevronLabel.layer.opacity = 1.0;
    self.buyLabel.layer.opacity = 1.0;
    self.buySizeView.layer.opacity = 1.0;
    self.dividerView.layer.opacity = 1.0;
    self.sizeButton.enabled = YES;
    self.buyButton.enabled = YES;
}

@end
