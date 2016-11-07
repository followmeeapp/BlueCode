//
//  BlueIntroScreenController.m
//  Follow
//
//  Created by Erich Ocean on 10/5/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueIntroScreenController.h"

#import <AVFoundation/AVFoundation.h>
#import <Crashlytics/Crashlytics.h>

#import "BlueApp.h"

@interface BlueIntroScreenController ()

@property (nonatomic, copy) NSString *videoName;
@property (nonatomic) AVPlayer *avPlayer;

@end

@implementation BlueIntroScreenController

- (instancetype)
initWithVideo: (NSString *)  videoName
status:        (IntroStatus) status
{
    if (self = [super init]) {
        self.status = status;
        self.videoName = videoName;
    }

    return self;
}

- (void) viewDidLoad
{
    [super viewDidLoad];

    [APP_DELEGATE.blueAnalytics viewTour: self.videoName];

    NSString *filepath = [[NSBundle mainBundle] pathForResource: self.videoName ofType: nil];
    NSURL *fileURL = [NSURL fileURLWithPath: filepath];
    self.avPlayer = [AVPlayer playerWithURL: fileURL];
    self.avPlayer.actionAtItemEnd = AVPlayerActionAtItemEndNone;

    AVPlayerLayer *videoLayer = [AVPlayerLayer playerLayerWithPlayer: self.avPlayer];
    videoLayer.frame = self.view.bounds;
    videoLayer.videoGravity = AVLayerVideoGravityResizeAspectFill;
    [self.view.layer addSublayer: videoLayer];

    CGRect frame = self.view.frame;

    UIButton *actionButton = [[UIButton alloc] initWithFrame: CGRectMake(0, frame.size.height - 100.0, frame.size.width, 50.0)];
    [actionButton addTarget: self action: @selector(tappedAction:) forControlEvents: UIControlEventTouchUpInside];
    [self.view addSubview: actionButton];

    UIButton *skipButton = [[UIButton alloc] initWithFrame: CGRectMake(0, frame.size.height - 50.0, frame.size.width, 50.0)];
    [skipButton addTarget: self action: @selector(tappedSkip:) forControlEvents: UIControlEventTouchUpInside];
    [self.view addSubview: skipButton];
}

- (void) reset
{
    [[self.avPlayer currentItem] seekToTime: kCMTimeZero];
}

- (void) play
{
    [self.avPlayer play];
}

- (void) pause
{
    [self.avPlayer pause];
}

- (IBAction) tappedAction: sender
{
    if (_status == LastScreen) {
        [self.introController createCard];

    } else {
        UIPageViewController *pvc = self.introController.pageViewController;
        BlueIntroScreenController *sc = (BlueIntroScreenController *)[self.introController pageViewController:                pvc
                                                                                           viewControllerAfterViewController: self];

        [self.introController pageViewController: pvc willTransitionToViewControllers: @[sc]];
        [pvc setViewControllers: @[sc] direction: UIPageViewControllerNavigationDirectionForward animated: NO completion: ^(BOOL didComplete) {
            [self.introController pageViewController:      self.introController.pageViewController
                                  didFinishAnimating:      didComplete
                                  previousViewControllers: @[self]
                                  transitionCompleted:     didComplete                            ];
        }];
    }
}

- (IBAction) tappedSkip: sender
{
    [self.introController skipIntro];
}

@end
