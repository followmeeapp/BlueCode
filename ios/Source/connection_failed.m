//
//  connection_failed.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Shared.h"
#import "Follow-private.h"
#define STATE_NAME CONNECTION_FAILED

static const int ddLogLevel __attribute__((unused)) = LOG_LEVEL_DEBUG;

@interface STATE : EOState

@property (nonatomic, strong) UIImageView *imageView;
@property (nonatomic, strong) UILabel *label;

@end

@implementation STATE

- (void) enterState
{
    UIView *contentView = ((ScreenState *)self.superstate).contentView;
    CGPoint center = contentView.center;
    
    self.imageView = [[UIImageView alloc] init];
    self.imageView.image = [UIImage imageNamed: @"warning-icon.png"];
    [self.imageView sizeToFit];

    CGRect frame  = self.imageView.frame;
    frame.size.width = frame.size.width / 2.0;
    frame.size.height = frame.size.height / 2.0;
    self.imageView.frame = frame;

    center.y = contentView.frame.size.height - 150;
    self.imageView.center = center;
    
    [contentView addSubview: self.imageView];

    self.label = [[UILabel alloc] init];
    self.label.text = @"No connection to server.";
    self.label.textColor = [UIColor whiteColor];
    [self.label sizeToFit];
    
    center.y = contentView.frame.size.height - 65;
    self.label.center = center;
    
    [contentView addSubview: self.label];
}

- (void) exitState
{
    [self.imageView removeFromSuperview];
    self.imageView = nil;

    [self.label removeFromSuperview];
    self.label = nil;
}

#pragma mark - Actions

- (void) webSocketStatusDidChange: (BOOL) hasWebSocket
{
    [self handled];
}

@end
