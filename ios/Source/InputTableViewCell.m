//
//  InputTableViewCell.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "InputTableViewCell.h"

#import <Atlas/Atlas.h>

@interface InputTableViewCell ()

@property (nonatomic) UILabel *guideLabel;
@property (nonatomic) NSLayoutConstraint *guideLabelLeftConstraint;

@end

@implementation InputTableViewCell

- (instancetype)
initWithStyle:   (UITableViewCellStyle) style
reuseIdentifier: (NSString *)           reuseIdentifier
{
    if (self = [super initWithStyle: style reuseIdentifier: reuseIdentifier]) {
        self.selectionStyle = UITableViewCellSelectionStyleNone;

        _textField = [[UITextField alloc] init];
        _textField.translatesAutoresizingMaskIntoConstraints = NO;
        _textField.returnKeyType = UIReturnKeyDone;
        _textField.font = [UIFont systemFontOfSize: 17];
        _textField.textColor = [UIColor darkGrayColor];
        [self.contentView addSubview: _textField];

        _guideLabel = [[UILabel alloc] init];
        _guideLabel.translatesAutoresizingMaskIntoConstraints = NO;
        _guideLabel.font = [UIFont systemFontOfSize: 17];
        _guideLabel.textColor = [UIColor darkGrayColor];
        [self.contentView addSubview: _guideLabel];

        [self configureLayoutConstraints];
    }

    return self;
}

- (void) updateConstraints
{
    self.guideLabelLeftConstraint.constant = self.separatorInset.left;
    [super updateConstraints];
}

- (void) configureLayoutConstraints
{
    [self.contentView addConstraint: [NSLayoutConstraint constraintWithItem: self.guideLabel
                                                         attribute:          NSLayoutAttributeCenterY
                                                         relatedBy:          NSLayoutRelationEqual
                                                         toItem:             self.contentView
                                                         attribute:          NSLayoutAttributeCenterY
                                                         multiplier:         1.0
                                                         constant:           0                       ]];

    self.guideLabelLeftConstraint = [NSLayoutConstraint constraintWithItem: self.guideLabel
                                                        attribute:          NSLayoutAttributeLeft
                                                        relatedBy:          NSLayoutRelationEqual
                                                        toItem:             self.contentView
                                                        attribute:          NSLayoutAttributeLeft
                                                        multiplier:         1.0
                                                        constant:           self.separatorInset.left];

    [self.contentView addConstraint: self.guideLabelLeftConstraint];

    [self.contentView addConstraint: [NSLayoutConstraint constraintWithItem: self.textField
                                                         attribute:          NSLayoutAttributeRight
                                                         relatedBy:          NSLayoutRelationEqual
                                                         toItem:             self.contentView
                                                         attribute:          NSLayoutAttributeRight
                                                         multiplier:         1.0
                                                         constant:           -10                   ]];

    [self.contentView addConstraint: [NSLayoutConstraint constraintWithItem: self.textField
                                                         attribute:          NSLayoutAttributeLeft
                                                         relatedBy:          NSLayoutRelationEqual
                                                         toItem:             self.guideLabel
                                                         attribute:          NSLayoutAttributeRight
                                                         multiplier:         1.0
                                                         constant:           10                    ]];

    [self.contentView addConstraint: [NSLayoutConstraint constraintWithItem: self.textField
                                                         attribute:          NSLayoutAttributeTop
                                                         relatedBy:          NSLayoutRelationEqual
                                                         toItem:             self.contentView
                                                         attribute:          NSLayoutAttributeTop
                                                         multiplier:         1.0
                                                         constant:           0                    ]];

    [self.contentView addConstraint:[NSLayoutConstraint constraintWithItem: self.textField
                                                        attribute:          NSLayoutAttributeHeight
                                                        relatedBy:          NSLayoutRelationEqual
                                                        toItem:             self.contentView
                                                        attribute:          NSLayoutAttributeHeight
                                                        multiplier:         1.0
                                                        constant:           0                      ]];
}

- (void) setSeparatorInset: (UIEdgeInsets) separatorInset
{
    [super setSeparatorInset: separatorInset];
    [self setNeedsUpdateConstraints];
}

- (void) setGuideText: (NSString *) guideText
{
    self.guideLabel.text = guideText;
    self.guideLabel.accessibilityLabel = guideText;
}

- (void) setPlaceHolderText: (NSString *) placeHolderText
{
    self.textField.accessibilityLabel = placeHolderText;
    self.textField.placeholder = placeHolderText;
}

@end
