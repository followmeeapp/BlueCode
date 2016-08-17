//
//  ScreenState.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Shared.h"
#import "ScreenState.h"

static const int ddLogLevel __attribute__((unused)) = LOG_LEVEL_DEBUG;

@interface ScreenState ()
@end

@implementation ScreenState

- (void) enterState
{
    [super enterState];

    UINib *xib = nil;

    if (self.xibName) {
        xib = [UINib nibWithNibName: self.xibName bundle: nil];

    } else {
        xib = [UINib nibWithNibName: self.stateName bundle: nil];
    }

    [xib instantiateWithOwner: self options: nil];

    NSAssert(self.contentView, @"ScreenState's contentView outlet not set.");

    [self.contentView setFrame: [UIScreen mainScreen].bounds];

//    [self.contentView setBackgroundColor: [UIColor colorWithRed: 247.0/256.0 green: 106.0/256.0 blue: 16.0/256.0 alpha: 1]];
//    [self.contentView setBackgroundColor: [UIColor whiteColor]];

//    if (_button1) [self addButtonGradient: _button1];
//    if (_button2) [self addButtonGradient: _button2];
//    if (_button3) [self addButtonGradient: _button3];
//    if (_cancelButton) [self addCancelButtonGradient: _cancelButton];

    [[[[UIApplication sharedApplication] delegate] window] rootViewController].view = self.contentView;
}

- (void) styleButton: (UIButton *) button
{
    button.layer.shadowColor = [[UIColor blackColor] CGColor];
    button.layer.shadowOpacity = 0.3f;
    button.layer.shadowPath = [[UIBezierPath bezierPathWithRect: button.bounds] CGPath];
    button.layer.shadowOffset = CGSizeMake(0.0, 4.0);
    button.layer.shadowRadius = 6.0f;
    
    button.layer.cornerRadius = 8.0f;
}

//- (void) addButtonGradient: (UIButton *) button
//{
//    button.layer.cornerRadius = 12.0; // half height & width
//    button.layer.borderWidth = 0.5;
//    button.layer.borderColor = [UIColor blackColor].CGColor;
//
////    CAGradientLayer *layer = [CAGradientLayer layer];
////    NSArray *colors = [NSArray arrayWithObjects: (id)[UIColor lightGrayColor].CGColor, (id)[UIColor darkGrayColor].CGColor, nil];
////    [layer setColors: colors];
////    [layer setFrame: button.bounds];
////    [button.layer insertSublayer:layer atIndex:0];
////    button.clipsToBounds = YES; // Important!
//
//    button.titleEdgeInsets = UIEdgeInsetsMake(4, 0, 0, 0);
//
//    button.titleLabel.font = [UIFont fontWithName: @"TradeGothicLTStd-Cn18" size: 24.0];
//    [button setTitleColor: [UIColor blackColor] forState: UIControlStateNormal];
//}
//
//- (void) addCancelButtonGradient: (UIButton *) button
//{
//    button.layer.cornerRadius = 7.0; // half height & width
//    button.layer.borderWidth = 1.0;
//    button.layer.borderColor = [UIColor blackColor].CGColor;
//
//    CAGradientLayer *layer = [CAGradientLayer layer];
//    NSArray *colors = [NSArray arrayWithObjects: (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, nil];
//    [layer setColors: colors];
//    [layer setFrame: button.bounds];
//    [button.layer insertSublayer:layer atIndex:0];
//    button.clipsToBounds = YES; // Important!
//
//    button.titleEdgeInsets = UIEdgeInsetsMake(4, 0, 0, 0);
//
//    button.titleLabel.font = [UIFont fontWithName: @"TradeGothicLTStd-BdCn20" size: 16.0];
//    [button setTitleColor: [UIColor darkGrayColor] forState: UIControlStateNormal];
//}

- (EOState *) defaultSubstate
{
    return nil;
}

- (void) configureState
{
    if ([self.xibName isEqualToString: @"TempScreen"]) {
        [self.titleLabel setText: NSStringFromClass([self class])];
    }
}

- (void) activateState
{

}

- (void) deactivateState
{

}

//- (void) exitState
//{
//    [_timer invalidate];
//    _timer = nil;
//}

@end
