//
//  NavigationController.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

@interface NavigationController : UINavigationController

@property (nonatomic, getter=isAnimating, readonly) BOOL animating;

- (void) notifyWhenCompletionEndsUsingBlock: (void (^)()) handler;

@end

@interface UIViewController (NavigationController)

@property (nonatomic, readonly) NavigationController *Follow_navigationController;

@end
