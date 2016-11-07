//
//  BlueIntroController.m
//  Follow
//
//  Created by Erich Ocean on 10/5/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueIntroController.h"

#import "BlueApp.h"

#import "BlueIntroScreenController.h"

@interface BlueIntroController ()

@property (nonatomic, assign) IntroStatus introStatus;

@property (nonatomic, strong) BlueIntroScreenController *screenController;
@property (nonatomic, strong) BlueIntroScreenController *pendingScreenController;

@end

@implementation BlueIntroController

- (void) viewDidLoad
{
    [super viewDidLoad];

    self.view.backgroundColor = APP_DELEGATE.grey;

    self.introStatus = FirstScreen;

    UIPageViewController *vc = [[UIPageViewController alloc] initWithTransitionStyle: UIPageViewControllerTransitionStyleScroll
                                                             navigationOrientation:   UIPageViewControllerNavigationOrientationHorizontal
                                                             options:                 nil                                                ];

    vc.delegate = self;
    vc.dataSource = self;

    self.pageViewController = vc;

    self.screenController = [[BlueIntroScreenController alloc] initWithVideo: [self videoNameForStatus: _introStatus] status: _introStatus];
    self.screenController.introController = self;
    [vc setViewControllers: @[self.screenController] direction: UIPageViewControllerNavigationDirectionForward animated: NO completion: nil];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self presentViewController: vc animated: NO completion: ^{
            [self.screenController play];
        }];
    });
}

- (NSString *) videoNameForStatus: (IntroStatus) status
{
    switch (status) {
        case FirstScreen:  return @"tour0.mp4";
        case SecondScreen: return @"tour1.mp4";
        case ThirdScreen:  return @"tour2.mp4";
        case FourthScreen: return @"tour3.mp4";
        case LastScreen:   return self.isUserInitiatedTour ? @"tour4b.mp4" : @"tour4.mp4";
        default: return nil;
    }
}

#pragma mark UIPageViewControllerDataSource

- (UIViewController *)
pageViewController:                 (UIPageViewController *) pageViewController
viewControllerBeforeViewController: (UIViewController *)     viewController
{
    if (_introStatus == FirstScreen) {
        return nil;

    } else {
        BlueIntroScreenController *vc = [[BlueIntroScreenController alloc] initWithVideo: [self videoNameForStatus: _introStatus - 1] status: _introStatus - 1];
        vc.introController = self;
        return vc;
    }
}

- (UIViewController *)
pageViewController:                (UIPageViewController *) pageViewController
viewControllerAfterViewController: (UIViewController *)     viewController
{
    if (_introStatus == LastScreen) {
        return nil;

    } else {
        BlueIntroScreenController *vc = [[BlueIntroScreenController alloc] initWithVideo: [self videoNameForStatus: _introStatus + 1] status: _introStatus + 1];
        vc.introController = self;
        return vc;
    }
}

// Uncomment this to show UIPageControl widget.
//- (NSInteger) presentationCountForPageViewController: (UIPageViewController *) pageViewController
//{
//    return LastScreen;
//}
//
//- (NSInteger) presentationIndexForPageViewController: (UIPageViewController *) pageViewController
//{
//    return _introStatus - 1;
//}

#pragma mark UIPageViewControllerDelegate

- (void)
pageViewController:              (UIPageViewController *)        pageViewController
willTransitionToViewControllers: (NSArray<UIViewController *> *) pendingViewControllers
{
    [self.screenController pause];

    BlueIntroScreenController *vc = (BlueIntroScreenController *)pendingViewControllers[0];
    self.pendingScreenController = vc;
}

- (void)
pageViewController:      (UIPageViewController *)        pageViewController
didFinishAnimating:      (BOOL)                          finished
previousViewControllers: (NSArray<UIViewController *> *) previousViewControllers
transitionCompleted:     (BOOL)                          completed
{
    self.screenController = self.pendingScreenController;
    self.introStatus = self.screenController.status;

    [self.screenController reset];
    [self.screenController play];
}

#pragma mark Actions

- (void) createCard
{
    [self.pageViewController dismissViewControllerAnimated: NO completion:^{
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
            if (self.isUserInitiatedTour) {
                [self dismissViewControllerAnimated: YES completion:^{

                }];

            } else {
                [APP_DELEGATE showMainAndCreateCard];
            }
        });
    }];
}

- (void) skipIntro
{
    [self.pageViewController dismissViewControllerAnimated: NO completion:^{
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
            if (self.isUserInitiatedTour) {
                [self dismissViewControllerAnimated: YES completion:^{

                }];

            } else {
                [APP_DELEGATE showMain];
            }
        });
    }];
}

@end
