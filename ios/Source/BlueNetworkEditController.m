//
//  BlueNetworkEditController.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueNetworkEditController.h"

#import "BlueApp.h"

#import <QuartzCore/QuartzCore.h>

#import "Blue-Swift.h"

#import "NetworkObject.h"

@interface BlueNetworkEditController () <UITextFieldDelegate>

@end

@implementation BlueNetworkEditController

- (void) viewDidLoad
{
    [super viewDidLoad];

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

    [self.textField becomeFirstResponder];
}

- (IBAction) testDeepLink: sender
{
    NSString *username = self.textField.text;

    if (!username || [username length] == 0) return;

    // TODO(Erich): This logic is very similar to that in BlueCardController.
    switch ((NetworkType)[self.networkKey integerValue]) {
        case PokemonGoType: {
            NSString *urlString = @"https://itunes.apple.com/us/app/pokemon-go/id1094591345";
            // Android: https://play.google.com/store/apps/details?id=com.nianticlabs.pokemongo
            [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
        } break;

        case FacebookType: {
            NSString *urlString = [NSString stringWithFormat: @"https://www.facebook.com/%@", username];
            [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
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
            // (or deep link with actual numeric id on iOS)
            NSString *urlString = [NSString stringWithFormat: @"https://plus.google.com/u/0/+%@", username];
            [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
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

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
