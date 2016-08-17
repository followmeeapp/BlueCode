//
//  connecting.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Shared.h"
#import "Follow-private.h"
#define STATE_NAME CONNECTING

static const int ddLogLevel __attribute__((unused)) = LOG_LEVEL_DEBUG;

@interface STATE : EOState

@property (nonatomic, strong) UIActivityIndicatorView *activityIndicatorView;

@end

@implementation STATE

- (void) enterState
{
    UIView *contentView = ((ScreenState *)self.superstate).contentView;
    
    self.activityIndicatorView = [[UIActivityIndicatorView alloc] init];
    [self.activityIndicatorView sizeToFit];
    self.activityIndicatorView.hidden = NO;
    [self.activityIndicatorView startAnimating];
    
    CGPoint center = contentView.center;
    center.y = contentView.frame.size.height - 100;
    self.activityIndicatorView.center = center;
    
    [contentView addSubview: self.activityIndicatorView];
}

- (void) exitState
{
    [self.activityIndicatorView removeFromSuperview];
    self.activityIndicatorView = nil;
}

@end
