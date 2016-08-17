//
//  BlueCardController.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueCardController.h"

#import <SDWebImage/UIImageView+WebCache.h>

#import "BlueApp.h"
#import "BlueModel.h"

#import "BlueCardEditController.h"
#import "BlueCardEditNavigationController.h"

#import "CardObject.h"
#import "NetworkObject.h"

#import "Blue-Swift.h"

@interface BlueCardController ()

@property (nonatomic, strong) NSDictionary *networkInfo;

@property (nonatomic, assign) NSInteger fastFollowIndex;

@property (nonatomic, assign) NSInteger version;
@property (nonatomic, strong) RLMNotificationToken *token;

@end

@implementation BlueCardController

- (void) setupNetworkInfo
{
    // TODO(Erich): This is extremely similar to code in BlueSocialNetworksController.
    self.networkInfo = @{
        @(PokemonGoType): @{
            @"property": @"hasPokemonGo",
            @"name": @"Pokémon Go",
            @"icon": @"pokemongo-tile",
            @"largeIcon": @"pokemon-large",
            @"color": [UIColor colorWithRed: (255.0/255.0) green: (255.0/255.0) blue: (255.0/255.0) alpha: 1.0],
            @"key": @(PokemonGoType),
        },
        @(FacebookType): @{
            @"property": @"hasFacebook",
            @"name": @"Facebook",
            @"icon": @"facebook-tile",
            @"largeIcon": @"facebook-large",
            @"color": [UIColor colorWithRed: (58.0/255.0) green: (87.0/255.0) blue: (154.0/255.0) alpha: 1.0],
            @"key": @(FacebookType),
        },
        @(TwitterType): @{
            @"property": @"hasTwitter",
            @"name": @"Twitter",
            @"icon": @"twitter-tile",
            @"largeIcon": @"twitter-large",
            @"color": [UIColor colorWithRed: (80.0/255.0) green: (170.0/255.0) blue: (241.0/255.0) alpha: 1.0],
            @"key": @(TwitterType),
        },
        @(InstagramType): @{
            @"property": @"hasInstagram",
            @"name": @"Instagram",
            @"icon": @"instagram-tile",
            @"largeIcon": @"instagram-large",
            @"color": [UIColor colorWithRed: (60.0/255.0) green: (113.0/255.0) blue: (157.0/255.0) alpha: 1.0],
            @"key": @(InstagramType),
        },
        @(SnapchatType): @{
            @"property": @"hasSnapchat",
            @"name": @"Snapchat",
            @"icon": @"snapchat-tile",
            @"largeIcon": @"snapchat-large",
            @"color": [UIColor colorWithRed: (254.0/255.0) green: (255.0/255.0) blue: (0.0/255.0) alpha: 1.0],
            @"key": @(SnapchatType),
        },
        @(GooglePlusType): @{
            @"property": @"hasGooglePlus",
            @"name": @"Google+",
            @"icon": @"googleplus-tile",
            @"largeIcon": @"googleplus-large",
            @"color": [UIColor colorWithRed: (225.0/255.0) green: (73.0/255.0) blue: (50.0/255.0) alpha: 1.0],
            @"key": @(GooglePlusType),
        },
        @(YouTubeType): @{
            @"property": @"hasYouTube",
            @"name": @"YouTube",
            @"icon": @"youtube-tile",
            @"largeIcon": @"youtube-large",
            @"color": [UIColor colorWithRed: (234.0/255.0) green: (39.0/255.0) blue: (27.0/255.0) alpha: 1.0],
            @"key": @(YouTubeType),
        },
        @(PinterestType): @{
            @"property": @"hasPinterest",
            @"name": @"Pinterest",
            @"icon": @"pinterest-tile",
            @"largeIcon": @"pinterest-large",
            @"color": [UIColor colorWithRed: (208.0/255.0) green: (26.0/255.0) blue: (31.0/255.0) alpha: 1.0],
            @"key": @(PinterestType),
        },
        @(TumblrType): @{
            @"property": @"hasTumblr",
            @"name": @"Tumblr",
            @"icon": @"tumblr-tile",
            @"largeIcon": @"tumblr-large",
            @"color": [UIColor colorWithRed: (52.0/255.0) green: (69.0/255.0) blue: (93.0/255.0) alpha: 1.0],
            @"key": @(TumblrType),
        },
        @(LinkedInType): @{
            @"property": @"hasLinkedIn",
            @"name": @"LinkedIn",
            @"icon": @"linkedin-tile",
            @"largeIcon": @"linkedin-large",
            @"color": [UIColor colorWithRed: (0.0/255.0) green: (116.0/255.0) blue: (182.0/255.0) alpha: 1.0],
            @"key": @(LinkedInType),
        },
        @(PeriscopeType): @{
            @"property": @"hasPeriscope",
            @"name": @"Periscope",
            @"icon": @"periscope-tile",
            @"largeIcon": @"periscope-large",
            @"color": [UIColor colorWithRed: (53.0/255.0) green: (163.0/255.0) blue: (198.0/255.0) alpha: 1.0],
            @"key": @(PeriscopeType),
        },
        @(VineType): @{
            @"property": @"hasVine",
            @"name": @"Vine",
            @"icon": @"vine-tile",
            @"largeIcon": @"vine-large",
            @"color": [UIColor colorWithRed: (0.0/255.0) green: (182.0/255.0) blue: (135.0/255.0) alpha: 1.0],
            @"key": @(VineType),
        },
        @(SoundCloudType): @{
            @"property": @"hasSoundCloud",
            @"name": @"SoundCloud",
            @"icon": @"soundcloud-tile",
            @"largeIcon": @"soundcloud-large",
            @"color": [UIColor colorWithRed: (255.0/255.0) green: (136.0/255.0) blue: (0.0/255.0) alpha: 1.0],
            @"key": @(SoundCloudType),
        },
        @(SinaWeiboType): @{
            @"property": @"hasSinaWeibo",
            @"name": @"Sina Weibo",
            @"icon": @"weibo-tile",
            @"largeIcon": @"weibo-large",
            @"color": [UIColor colorWithRed: (182.0/255.0) green: (48.0/255.0) blue: (47.0/255.0) alpha: 1.0],
            @"key": @(SinaWeiboType),
        },
        @(VKontakteType): @{
            @"property": @"hasVKontakte",
            @"name": @"VKontakte",
            @"icon": @"vk-tile",
            @"largeIcon": @"vk-large",
            @"color": [UIColor colorWithRed: (75.0/255.0) green: (117.0/255.0) blue: (163.0/255.0) alpha: 1.0],
            @"key": @(VKontakteType),
        },
    };
}

- (void) updateAvatar
{
    CardObject *card = self.card;
    if (!card) return;

    NSString *team = nil;

    for (NSInteger idx=0, len=card.networks.count; idx<len; ++idx) {
        NetworkObject *network = card.networks[idx];
        if (network.type == PokemonGoType) {
            team = network.username;
            break;
        }
    }

    CALayer *layer = self.avatar.layer;

    if (!team) {
        layer.borderColor = [UIColor blackColor].CGColor;
        layer.borderWidth = 1.0;

    } else if ([team isEqualToString: @"instinct"]) {
        layer.borderColor = [APP_DELEGATE instinctColor].CGColor;
        layer.borderWidth = 12.0;

    } else if ([team isEqualToString: @"mystic"]) {
        layer.borderColor = [APP_DELEGATE mysticColor].CGColor;
        layer.borderWidth = 12.0;

    } else if ([team isEqualToString: @"valor"]) {
        layer.borderColor = [APP_DELEGATE valorColor].CGColor;
        layer.borderWidth = 12.0;
    }
}

- (void) display
{
    CardObject *card = self.card;
    if (!card) return;

    self.avatar.clipsToBounds = YES;
    self.avatar.layer.cornerRadius = 78; // this value vary as per your desire
    self.avatar.layer.borderColor = [UIColor whiteColor].CGColor;
    self.avatar.layer.borderWidth = 1.0;

    [self updateAvatar];

    if (card.avatarURLString) {
        [self.avatar sd_setImageWithURL: [NSURL URLWithString: card.avatarURLString]
                       placeholderImage:   [UIImage imageNamed: @"missing-avatar"]         ];

    } else {
        self.avatar.image = [UIImage imageNamed: @"missing-avatar"];
    }

    if (card.backgroundURLString) {
        [self.background sd_setImageWithURL: [NSURL URLWithString: card.backgroundURLString]
                           placeholderImage:   [UIImage imageNamed: @"default-cover-photo"]  ];

    } else {
        self.background.image = [UIImage imageNamed: @"default-cover-photo"];
    }

    self.fullName.text = card.fullName;
    self.location.text = card.location ? card.location : @"";
    self.bio.text = card.bio ? card.bio : @"";

    self.editButton.hidden = (card.id != [APP_DELEGATE.blueModel activeUserCard].id);

    self.fastFollowButton.layer.borderColor = [UIColor whiteColor].CGColor;
    self.fastFollowButton.layer.borderWidth = 1.0;
    self.fastFollowButton.layer.cornerRadius = 5.0;

    self.version = card.version;

    [self.networks reloadData];
}

- (void) viewDidLoad
{
    [super viewDidLoad];

    self.fastFollowIndex = -1;

    [self setupNetworkInfo];

    [self display];

    RLMRealm *realm = [RLMRealm defaultRealm];
    self.token = [realm addNotificationBlock: ^(NSString *notification, RLMRealm *realm) {
        CardObject *card = self.card;
        if (!card) return;

        if (card.version > self.version) [self display];
    }];
}

- (void) dealloc
{
    if (self.token) {
        [self.token stop];
        self.token = nil;
    }
}

- (IBAction) dismiss: (id)sender
{
    [self dismissViewControllerAnimated: YES completion: nil];
}

- (NSInteger) numberOfSectionsInCollectionView: (UICollectionView *) collectionView
{
    return 1;
}

-(NSInteger)
collectionView:         (UICollectionView *) collectionView
numberOfItemsInSection: (NSInteger)          section
{
    return self.card.networks.count;
}

- (UICollectionViewCell *)
collectionView:         (UICollectionView *) collectionView
cellForItemAtIndexPath: (NSIndexPath *)      indexPath
{
    NetworkObject *network = [self.card.networks objectAtIndex: indexPath.row];

    static NSString *cellIdentifier = @"CardNetworkCell";

    UICollectionViewCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier: cellIdentifier forIndexPath: indexPath];

    UIImageView *imageView = (UIImageView *)[cell viewWithTag: 1];
    imageView.image = [UIImage imageNamed: self.networkInfo[@(network.type)][@"icon"]];

    return cell;
}

- (void)
collectionView:           (UICollectionView *) collectionView
didSelectItemAtIndexPath: (NSIndexPath *)      indexPath
{
    CardObject *card = self.card;
    if (!card) return;

    NetworkObject *network = [self.card.networks objectAtIndex: indexPath.row];
    if (!network) return;

    [self launchAppForCard: card network: network onlyIfUserHasNetwork: NO];
}

- (BOOL)
launchAppForCard:     (CardObject *)    card
network:              (NetworkObject *) network
onlyIfUserHasNetwork: (BOOL)            onlyIfUserHasNetwork
{
    NSDictionary *networkInfo = self.networkInfo[@(network.type)];

    if (![card[networkInfo[@"property"]] boolValue]) {
        return NO; // Card does not have this network set up.
    }

    if (network.type == PokemonGoType && onlyIfUserHasNetwork) {
        return NO; // We don't "fast follow" Pokémon Go.
    }

    CardObject *userCard = [APP_DELEGATE.blueModel activeUserCard];
    if (!userCard) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Members only."
                                                      message:           @"You need to Join Blue before you can view other user's cards."
                                                      delegate:          nil
                                                      cancelButtonTitle: @"OK"
                                                      otherButtonTitles: nil                                                            ];
        [alertView show];
        return onlyIfUserHasNetwork ? YES : NO;
#pragma clang diagnostic pop

    } else {
        if (![userCard[networkInfo[@"property"]] boolValue]) {
            if (onlyIfUserHasNetwork) {
                return NO; // User does not have this social network configured.

            } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                // TODO(Erich): Allow the user to go to the App Store to install the app, or edit their own card.
                UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Members only."
                                                              message:           @"You don't have this social network enabled in your own card."
                                                              delegate:          nil
                                                              cancelButtonTitle: @"OK"
                                                              otherButtonTitles: nil                                                           ];
                [alertView show];
                return NO;
#pragma clang diagnostic pop
            }
        }
    }

    switch (network.type) {
        case PokemonGoType: {
            NSString *urlString = @"https://itunes.apple.com/us/app/pokemon-go/id1094591345";
            // Android: https://play.google.com/store/apps/details?id=com.nianticlabs.pokemongo
            [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
        } break;

        case FacebookType: {
            if (!card.hasFacebook) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == FacebookType) {
                    NSString *urlString = [NSString stringWithFormat: @"https://www.facebook.com/%@", network.username];
                    [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
                }
            }
        } break;

        case TwitterType: {
            if (!card.hasTwitter) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == TwitterType) {
                    id instance = [[BlueDeepLink alloc] init];
                    [instance openTwitter: network.username];
                }
            }
        } break;

        case InstagramType: {
            if (!card.hasInstagram) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == InstagramType) {
                    id instance = [[BlueDeepLink alloc] init];
                    [instance openInstagram: network.username];
                }
            }
        }
        case SnapchatType: {
            if (!card.hasSnapchat) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == SnapchatType) {
                    id instance = [[BlueDeepLink alloc] init];
                    [instance openSnapchat: network.username];
                }
            }
        } break;

        case GooglePlusType: {
            if (!card.hasGooglePlus) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == GooglePlusType) {
                    // (or deep link with actual numeric id on iOS)
                    NSString *urlString = [NSString stringWithFormat: @"https://plus.google.com/u/0/+%@", network.username];
                    [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
                }
            }
        } break;

        case YouTubeType: {
            if (!card.hasYouTube) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == YouTubeType) {
                    NSString *urlString = [NSString stringWithFormat: @"https://www.youtube.com/user/%@", network.username];
                    [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
                }
            }
        } break;

        case PinterestType: {
            if (!card.hasPinterest) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == PinterestType) {
                    id instance = [[BlueDeepLink alloc] init];
                    [instance openPinterest: network.username];
                }
            }
        } break;

        case TumblrType: {
            if (!card.hasTumblr) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == TumblrType) {
                    NSString *urlString = [NSString stringWithFormat: @"http://%@.tumblr.com/", network.username];
                    [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
                }
            }
        } break;

        case LinkedInType: {
            if (!card.hasLinkedIn) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == LinkedInType) {
                    NSString *urlString = [NSString stringWithFormat: @"https://www.linkedin.com/in/%@", network.username];
                    [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
                }
            }
        } break;

        case PeriscopeType: {
            if (!card.hasPeriscope) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == PeriscopeType) {
                    NSString *urlString = [NSString stringWithFormat: @"https://www.periscope.tv/%@", network.username];
                    [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
                }
            }
        } break;

        case VineType: {
            if (!card.hasVine) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == VineType) {
                    NSString *urlString = [NSString stringWithFormat: @"https://vine.co/%@", network.username];
                    [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
                }
            }
        } break;

        case SoundCloudType: {
            if (!card.hasSoundCloud) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == SoundCloudType) {
                    NSString *urlString = [NSString stringWithFormat: @"https://soundcloud.com/%@", network.username];
                    [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
                }
            }
        } break;

        case SinaWeiboType: {
            if (!card.hasSinaWeibo) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == SinaWeiboType) {
                    NSString *urlString = [NSString stringWithFormat: @"http://weibo.com/%@", network.username];
                    [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
                }
            }
        } break;

        case VKontakteType: {
            if (!card.hasVKontakte) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == VKontakteType) {
                    NSString *urlString = [NSString stringWithFormat: @"https://vk.com/%@", network.username];
                    [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
                }
            }
        } break;
    }

    return YES; // We did a deep link.
}

- (IBAction) editCard: sender
{
    CardObject *card = [APP_DELEGATE.blueModel activeUserCard];
    if (self.card.id != card.id) return;

    UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
    BlueCardEditController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueCardEditController"];
    BlueCardEditNavigationController *nc = [[BlueCardEditNavigationController alloc] initWithRootViewController: vc];

    APP_DELEGATE.isUpdatingCard = YES;
    APP_DELEGATE.card = card;

    if (card.avatarURLString) {
        // FIXME(Erich): This only works with internet access! And it blocks the UI.
        APP_DELEGATE.savedAvatar = [UIImage imageWithData: [NSData dataWithContentsOfURL: [NSURL URLWithString: card.avatarURLString]]];

    } else {
        APP_DELEGATE.savedAvatar = nil;
    }

    if (card.backgroundURLString) {
        // FIXME(Erich): This only works with internet access! And it blocks the UI.
        APP_DELEGATE.savedCoverPhoto = [UIImage imageWithData: [NSData dataWithContentsOfURL: [NSURL URLWithString: card.backgroundURLString]]];

    } else {
        APP_DELEGATE.savedCoverPhoto = nil;
    }

    APP_DELEGATE.fullName = card.fullName;
    APP_DELEGATE.location = card.location;
    APP_DELEGATE.bio = card.bio;

    NSMutableDictionary *networks = [NSMutableDictionary dictionary];
    for (NSInteger idx=0, len=card.networks.count; idx<len; ++idx) {
        NetworkObject *network = card.networks[idx];
        networks[@(network.type)] = network.username;
    }

    APP_DELEGATE.networks = networks;

    [self presentViewController: nc animated: YES completion: nil];
}

- (IBAction) fastFollow: sender
{
    CardObject *userCard = [APP_DELEGATE.blueModel activeUserCard];
    if (!userCard) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Members only."
                                                      message:           @"You need to Join Blue before you can FastFollow another user's social networks."
                                                      delegate:          nil
                                                      cancelButtonTitle: @"OK"
                                                      otherButtonTitles: nil                                                            ];
        [alertView show];
        return;
#pragma clang diagnostic pop
    }

    self.fastFollowIndex = 0;

    [[NSNotificationCenter defaultCenter] addObserver: self
                                          selector:    @selector(continueFastFollow:)
                                          name:        @"ContinueFastFollowIfNeeded"
                                          object:      nil                          ];

    [self continueFastFollow: nil];
}

- (void) continueFastFollow: (NSNotification *) note
{
    if (self.fastFollowIndex < 0) {
        [[NSNotificationCenter defaultCenter] removeObserver: self];
        return;
    }

    CardObject *card = self.card;
    if (!card) {
        [[NSNotificationCenter defaultCenter] removeObserver: self];
        self.fastFollowIndex = -1;
        return;
    }

    if (self.fastFollowIndex >= self.card.networks.count) {
        [[NSNotificationCenter defaultCenter] removeObserver: self];
        self.fastFollowIndex = -1;
        return;
    }

    NetworkObject *network = [self.card.networks objectAtIndex: self.fastFollowIndex];
    if (!network) {
        [[NSNotificationCenter defaultCenter] removeObserver: self];
        self.fastFollowIndex = -1;
        return;
    }

    self.fastFollowIndex++;

    while (![self launchAppForCard: self.card network: network onlyIfUserHasNetwork: YES]) {
        if (self.fastFollowIndex >= self.card.networks.count) {
            [[NSNotificationCenter defaultCenter] removeObserver: self];
            self.fastFollowIndex = -1;
            return;
        }

        network = [self.card.networks objectAtIndex: self.fastFollowIndex];
        if (!network) {
            [[NSNotificationCenter defaultCenter] removeObserver: self];
            self.fastFollowIndex = -1;
            return;
        }

        self.fastFollowIndex++;
    }
}

@end
