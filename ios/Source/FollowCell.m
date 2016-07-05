//
//  FollowCell.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "FollowCell.h"

#define CELL_NAME @"FollowCell"

@implementation FollowCell

- (instancetype) initWithFrame: (CGRect) aRect
{
    if (self = [super initWithFrame: aRect]) {
        UINib *cellNib = [UINib nibWithNibName: CELL_NAME bundle: nil];
        [cellNib instantiateWithOwner: self options: nil];

        [self.cellView setTranslatesAutoresizingMaskIntoConstraints: YES];
        [self.contentView addSubview: self.cellView];
    }

    return self;
}

@end
