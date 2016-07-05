//
//  ChatMessageCell.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "ChatMessageCell.h"

@interface ChatMessageCell ()

@property (nonatomic) UIImageView *messageImageView;

@end

@implementation ChatMessageCell

- (id) initWithCoder: (NSCoder *) aDecoder
{
    if (self = [super initWithCoder: aDecoder]) {
        self.messageImageView = [[UIImageView alloc] init];
        self.messageImageView.tag = 1;
        self.messageImageView.frame = CGRectMake(100, 30, 150, 90);

        [self addSubview: self.messageImageView];
    }

    return self;
}

- (void) updateWithImage: (UIImage *) image
{
    self.messageImageView.image = image;
}

- (void) removeImage
{
    self.messageImageView.image = nil;
}

- (void) assignText: (NSString *) text
{
    self.messageLabel.text = text;
}

@end
