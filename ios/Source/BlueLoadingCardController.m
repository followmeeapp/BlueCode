//
//  BlueLoadingCardController.m
//  Follow
//
//  Created by Erich Ocean on 10/23/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueLoadingCardController.h"

#import "BlueApp.h"

#import "CardObject.h"

@implementation BlueLoadingCardController

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

    self.fullName.text = self.card.fullName;
    self.location.text = self.card.location;
}

- (IBAction) dismiss: sender
{
    [self dismissViewControllerAnimated: YES completion: nil];
}

@end
