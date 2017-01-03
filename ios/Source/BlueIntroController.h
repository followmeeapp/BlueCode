//
//  BlueIntroController.h
//  Follow
//
//  Created by Erich Ocean on 10/5/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

typedef NS_ENUM(NSInteger, IntroStatus) {
    IntroStatusUnknown = 0,
    FirstScreen,
    SecondScreen,
    ThirdScreen,
    FourthScreen,
    LastScreen,
};

@interface BlueIntroController : UIViewController <UIPageViewControllerDataSource, UIPageViewControllerDelegate>

@property (nonatomic, assign) BOOL delayShowingIntro;

@property (nonatomic, assign) BOOL isUserInitiatedTour;

@property (nonatomic, strong) UIPageViewController *pageViewController;

- (void) createCard;
- (void) skipIntro;

- (void) showIntro;

@end
