//
//  CenterTextTableViewCell.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "CenterTextTableViewCell.h"

@implementation CenterTextTableViewCell

- (instancetype)
initWithStyle:   (UITableViewCellStyle) style
reuseIdentifier: (NSString *)           reuseIdentifier
{
    if (self = [super initWithStyle:style reuseIdentifier:reuseIdentifier]) {
        self.centerTextLabel = [[UILabel alloc] init];
        self.centerTextLabel.translatesAutoresizingMaskIntoConstraints = NO;
        self.centerTextLabel.textAlignment = NSTextAlignmentCenter;
        [self.contentView addSubview: self.centerTextLabel];

        [self setUpConstraints];
    }

    return self;
}

- (void) setUpConstraints
{
    [self addConstraint: [NSLayoutConstraint constraintWithItem: self.centerTextLabel
                                             attribute:          NSLayoutAttributeLeft
                                             relatedBy:          NSLayoutRelationEqual
                                             toItem:             self.contentView
                                             attribute:          NSLayoutAttributeLeft
                                             multiplier:         1.0
                                             constant:           10.0                 ]];

    [self addConstraint: [NSLayoutConstraint constraintWithItem: self.centerTextLabel
                                             attribute:          NSLayoutAttributeRight
                                             relatedBy:          NSLayoutRelationEqual
                                             toItem:             self.contentView
                                             attribute:          NSLayoutAttributeRight
                                             multiplier:         1.0
                                             constant:           -10.0                 ]];

    [self addConstraint: [NSLayoutConstraint constraintWithItem: self.centerTextLabel
                                             attribute:          NSLayoutAttributeCenterY
                                             relatedBy:          NSLayoutRelationEqual
                                             toItem:             self.contentView
                                             attribute:          NSLayoutAttributeCenterY
                                             multiplier:         1.0
                                             constant:           0.0                     ]];
}

@end
