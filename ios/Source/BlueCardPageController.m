//
//  BlueCardPageController.m
//  Follow
//
//  Created by Erich Ocean on 10/23/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueCardPageController.h"

#import "BlueApp.h"

@interface BlueCardPageController ()

@end

@implementation BlueCardPageController

- (void) viewDidLoad
{
    [super viewDidLoad];

    // This is meant to appear identical to the navigation bar in BlueMainController.
    UIImage *image = [UIImage imageNamed: @"navbar-icon"];
    UIButton *titleButton = [UIButton buttonWithType: UIButtonTypeCustom];
    [titleButton setImage: image forState: UIControlStateNormal];
    [titleButton setUserInteractionEnabled: NO];
    [titleButton setFrame: CGRectMake(0, 0, 116, 53.0)];
    self.navigationItem.titleView = titleButton;

    UIColor *tintColor = APP_DELEGATE.black;
    [self.navigationController.navigationBar setTintColor: tintColor];
    [self.navigationController.navigationBar setTitleTextAttributes: @{ NSForegroundColorAttributeName: tintColor }];

    image = [UIImage imageNamed: @"close-icon"];
    UIBarButtonItem *menuButton = [[UIBarButtonItem alloc] initWithImage: image
                                                           style:         UIBarButtonItemStylePlain
                                                           target:        self
                                                           action:        @selector(dismiss:)      ];
    self.navigationItem.rightBarButtonItem = menuButton;
}

- (IBAction) dismiss: sender
{
    [self dismissViewControllerAnimated: YES completion: nil];
}

@end
