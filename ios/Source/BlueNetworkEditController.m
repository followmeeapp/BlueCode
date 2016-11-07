//
//  BlueNetworkEditController.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueNetworkEditController.h"

#import <Crashlytics/Crashlytics.h>
#import <QuartzCore/QuartzCore.h>

#import "BlueApp.h"

#import "BlueWebController.h"

#import "Blue-Swift.h"

#import "NetworkObject.h"

@interface BlueNetworkEditController () <UITextFieldDelegate>

@end

@implementation BlueNetworkEditController

- (void) viewDidLoad
{
    [super viewDidLoad];

    self.navigationItem.title = self.networkName;

    self.button.clipsToBounds = YES;
    self.button.layer.cornerRadius = 4; // this value vary as per your desire
    self.button.layer.borderColor = [UIColor whiteColor].CGColor;
    self.button.layer.borderWidth = 1.0;

    self.view.backgroundColor = self.backgroundColor;

    self.imageView.image = self.image;

    self.textField.placeholder = self.promptText;

    NSString *username = APP_DELEGATE.networks[self.networkKey];
    if (username) {
        self.textField.text = username;
    }

    self.textField.delegate = self;

    if ([self.networkName isEqualToString: @"Snapchat"]) {
        [self.button setTitleColor: [UIColor blackColor] forState: UIControlStateNormal];
        self.button.layer.borderColor = [UIColor blackColor].CGColor;

        self.textField.textColor = [UIColor blackColor];

        self.divider.backgroundColor = [UIColor blackColor];
    }

    [self.textField becomeFirstResponder];

    UIImage *image = [UIImage imageNamed: @"tour-icon"];
    UIBarButtonItem *menuButton = [[UIBarButtonItem alloc] initWithImage: image
                                                           style:         UIBarButtonItemStylePlain
                                                           target:        self
                                                           action:        @selector(showHelp:)     ];
    self.navigationItem.rightBarButtonItem = menuButton;
}

- (void) viewWillDisappear: (BOOL) animated
{
    if ([self.navigationController.viewControllers indexOfObject: self] == NSNotFound) {
        [[NSNotificationCenter defaultCenter] postNotificationName: @"BlueRefreshNetworkIcons" object: nil];
    }

    [super viewWillDisappear: animated];
}

- (IBAction) showHelp: sender
{
    UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
    BlueWebController *wc = [sb instantiateViewControllerWithIdentifier: @"BlueWeb"];

    NSURL *url = [NSURL URLWithString: [NSString stringWithFormat: @"http://blue.social/help-%@", self.networkSlug]];
    NSLog(@"URL: %@", url);
    wc.url = url;

    [self.navigationController pushViewController: wc animated: YES];
}

- (IBAction) testDeepLink: sender
{
    NSString *username = self.textField.text;

    if (!username || [username length] == 0) return;

    [APP_DELEGATE.blueAnalytics testSocialNetwork: self.networkName
                                withUsername:      username
                                fromCard:          self.cardId     ];

    // TODO(Erich): This logic is very similar to that in BlueCardController.
    switch ((NetworkType)[self.networkKey integerValue]) {
        case FacebookType: {
            NSString *urlString = [NSString stringWithFormat: @"fb://profile?id=%@", username];
            NSURL *appURL = [NSURL URLWithString: urlString];
            if (![[UIApplication sharedApplication] openURL: appURL]) {
                urlString = [NSString stringWithFormat: @"https://www.facebook.com/%@", username];
                NSURL *webURL = [NSURL URLWithString: urlString];
                [[UIApplication sharedApplication] openURL: webURL];
            }
        } break;

        case TwitterType: {
            id instance = [[BlueDeepLink alloc] init];
            [instance openTwitter: username];
        } break;

        case InstagramType: {
            id instance = [[BlueDeepLink alloc] init];
            [instance openInstagram: username];
        }
        case SnapchatType: {
            id instance = [[BlueDeepLink alloc] init];
            [instance openSnapchat: username];
        } break;

        case GooglePlusType: {
            NSCharacterSet *notDigits = [[NSCharacterSet decimalDigitCharacterSet] invertedSet];
            if ([username rangeOfCharacterFromSet: notDigits].location == NSNotFound) {
                // TODO(Erich): we can deep link with an actual numeric id on iOS
                NSString *urlString = [NSString stringWithFormat: @"https://plus.google.com/%@", username];
                [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];

            } else {
                NSString *urlString = [NSString stringWithFormat: @"https://plus.google.com/u/0/+%@", username];
                [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
            }
        } break;

        case YouTubeType: {
            NSString *urlString = [NSString stringWithFormat: @"https://www.youtube.com/user/%@", username];
            [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
        } break;

        case PinterestType: {
            id instance = [[BlueDeepLink alloc] init];
            [instance openPinterest: username];
        } break;

        case TumblrType: {
            NSString *urlString = [NSString stringWithFormat: @"http://%@.tumblr.com/", username];
            [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
        } break;

        case LinkedInType: {
            NSString *urlString = [NSString stringWithFormat: @"https://www.linkedin.com/in/%@", username];
            [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
        } break;

        case PeriscopeType: {
            NSString *urlString = [NSString stringWithFormat: @"https://www.periscope.tv/%@", username];
            [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
        } break;

        case VineType: {
            NSString *urlString = [NSString stringWithFormat: @"https://vine.co/%@", username];
            [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
        } break;

        case SoundCloudType: {
            NSString *urlString = [NSString stringWithFormat: @"https://soundcloud.com/%@", username];
            [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
        } break;

        case SinaWeiboType: {
            NSString *urlString = [NSString stringWithFormat: @"http://weibo.com/%@", username];
            [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
        } break;

        case VKontakteType: {
            NSString *urlString = [NSString stringWithFormat: @"https://vk.com/%@", username];
            [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
        } break;
    }
}

- (BOOL) textFieldShouldReturn: (UITextField *) textField
{
    [textField resignFirstResponder];

    [self.navigationController popViewControllerAnimated: YES];

    return NO;
}

- (void) textFieldDidEndEditing: (UITextField *) textField
{
    NSString *username = textField.text;

    if (!username || [username length] == 0) {
        [APP_DELEGATE.networks removeObjectForKey: self.networkKey];

    } else {
        APP_DELEGATE.networks[self.networkKey] = username;
    }
}

@end
