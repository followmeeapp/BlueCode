//
//  NavigationController.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "NavigationController.h"

@interface NavigationController () <UINavigationControllerDelegate>

@property (nonatomic) NSMutableArray *animationCompletionHandlers;
@property (nonatomic, getter=isAnimating) BOOL animating;

@end

@implementation NavigationController

- (instancetype)
initWithNibName: (NSString *) nibNameOrNil
bundle:          (NSBundle *) nibBundleOrNil
{
    if (self = [super initWithNibName: nibNameOrNil bundle: nibBundleOrNil]) {
        self.delegate = self;
        _animationCompletionHandlers = [NSMutableArray new];
    }

    return self;
}

- (void) setDelegate: (id <UINavigationControllerDelegate>) delegate
{
    if (delegate != self) [NSException raise: NSInternalInconsistencyException format: @"NavigationController must act as its own delegate."];
    [super setDelegate: delegate];
}

- (void) notifyWhenCompletionEndsUsingBlock: (void (^)())handler
{
    if (!handler) return;

    if (!self.isAnimating) {
        handler();
        return;
    }

    [self.animationCompletionHandlers addObject: [handler copy]];
}

#pragma mark - UINavigationControllerDelegate

- (void)
navigationController:   (UINavigationController *) navigationController
willShowViewController: (UIViewController *)       viewController
animated:               (BOOL)                     animated
{
    if (!animated) return;
    self.animating = YES;

    [[self transitionCoordinator] animateAlongsideTransition: nil completion: ^(id<UIViewControllerTransitionCoordinatorContext> context) {
        if (![context isCancelled]) return;
        self.animating = NO;
        [self notifyAnimationCompletionHandlers];
    }];
}

- (void)
navigationController:  (UINavigationController *) navigationController
didShowViewController: (UIViewController *)       viewController
animated:              (BOOL)                     animated
{
    self.animating = NO;
    [self notifyAnimationCompletionHandlers];
}

#pragma mark - Helpers

- (void) notifyAnimationCompletionHandlers
{
    while (!self.isAnimating && self.animationCompletionHandlers.count > 0) {
        void (^handler)() = self.animationCompletionHandlers.firstObject;
        [self.animationCompletionHandlers removeObjectAtIndex:0];
        handler();
    }
}

@end

@implementation UIViewController (NavigationController)

- (NavigationController *) Follow_navigationController
{
    if (![self.navigationController isKindOfClass: NavigationController.class]) return nil;

    return (NavigationController *)self.navigationController;
}

@end
