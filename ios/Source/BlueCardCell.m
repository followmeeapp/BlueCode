//
//  BlueCardCell.m
//  Follow
//
//  Created by Erich Ocean on 7/30/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueCardCell.h"

#import "NetworkObject.h"

@interface BlueCardCell ()

@property (nonatomic, strong) NSDictionary *networkInfo;

@end

@implementation BlueCardCell

- (void) awakeFromNib
{
    [super awakeFromNib];

    self.networks = @[];

    [self configureNetworkInfo];
}

#pragma mark UICollectionViewDataSource

- (NSInteger) numberOfSectionsInCollectionView: (UICollectionView *) collectionView
{
    return 1;
}

-(NSInteger)
collectionView:         (UICollectionView *) collectionView
numberOfItemsInSection: (NSInteger)          section
{
    return self.networks.count;
}

- (UICollectionViewCell *)
collectionView:         (UICollectionView *) collectionView
cellForItemAtIndexPath: (NSIndexPath *)      indexPath
{
    NSNumber *networkType = [self.networks objectAtIndex: indexPath.row];
    NSDictionary *props = self.networkInfo[networkType];

    static NSString *cellIdentifier = @"CardNetworkCell";
    UICollectionViewCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier: cellIdentifier forIndexPath: indexPath];

    UIImageView *iconView = (UIImageView *)[cell viewWithTag: 1];
    iconView.image = [UIImage imageNamed: props[@"icon"]];

    return cell;
}

#pragma mark Misc

- (void) configureNetworkInfo
{
    self.networkInfo = @{
        @(FacebookType): @{
            @"property": @"hasFacebook",
            @"name": @"Facebook",
            @"icon": @"facebook-icon",
            @"largeIcon": @"facebook-large",
            @"color": [UIColor colorWithRed: (58.0/255.0) green: (87.0/255.0) blue: (154.0/255.0) alpha: 1.0],
            @"key": @(FacebookType),
        },
        @(TwitterType): @{
            @"property": @"hasTwitter",
            @"name": @"Twitter",
            @"icon": @"twitter-icon",
            @"largeIcon": @"twitter-large",
            @"color": [UIColor colorWithRed: (80.0/255.0) green: (170.0/255.0) blue: (241.0/255.0) alpha: 1.0],
            @"key": @(TwitterType),
        },
        @(InstagramType): @{
            @"property": @"hasInstagram",
            @"name": @"Instagram",
            @"icon": @"instagram-icon",
            @"largeIcon": @"instagram-large",
            @"color": [UIColor colorWithRed: (60.0/255.0) green: (113.0/255.0) blue: (157.0/255.0) alpha: 1.0],
            @"key": @(InstagramType),
        },
        @(SnapchatType): @{
            @"property": @"hasSnapchat",
            @"name": @"Snapchat",
            @"icon": @"snapchat-icon",
            @"largeIcon": @"snapchat-large",
            @"color": [UIColor colorWithRed: (254.0/255.0) green: (255.0/255.0) blue: (0.0/255.0) alpha: 1.0],
            @"key": @(SnapchatType),
        },
        @(GooglePlusType): @{
            @"property": @"hasGooglePlus",
            @"name": @"Google+",
            @"icon": @"googleplus-icon",
            @"largeIcon": @"googleplus-large",
            @"color": [UIColor colorWithRed: (225.0/255.0) green: (73.0/255.0) blue: (50.0/255.0) alpha: 1.0],
            @"key": @(GooglePlusType),
        },
        @(YouTubeType): @{
            @"property": @"hasYouTube",
            @"name": @"YouTube",
            @"icon": @"youtube-icon",
            @"largeIcon": @"youtube-large",
            @"color": [UIColor colorWithRed: (234.0/255.0) green: (39.0/255.0) blue: (27.0/255.0) alpha: 1.0],
            @"key": @(YouTubeType),
        },
        @(PinterestType): @{
            @"property": @"hasPinterest",
            @"name": @"Pinterest",
            @"icon": @"pinterest-icon",
            @"largeIcon": @"pinterest-large",
            @"color": [UIColor colorWithRed: (208.0/255.0) green: (26.0/255.0) blue: (31.0/255.0) alpha: 1.0],
            @"key": @(PinterestType),
        },
        @(TumblrType): @{
            @"property": @"hasTumblr",
            @"name": @"Tumblr",
            @"icon": @"tumblr-icon",
            @"largeIcon": @"tumblr-large",
            @"color": [UIColor colorWithRed: (52.0/255.0) green: (69.0/255.0) blue: (93.0/255.0) alpha: 1.0],
            @"key": @(TumblrType),
        },
        @(LinkedInType): @{
            @"property": @"hasLinkedIn",
            @"name": @"LinkedIn",
            @"icon": @"linkedin-icon",
            @"largeIcon": @"linkedin-large",
            @"color": [UIColor colorWithRed: (0.0/255.0) green: (116.0/255.0) blue: (182.0/255.0) alpha: 1.0],
            @"key": @(LinkedInType),
        },
        @(PeriscopeType): @{
            @"property": @"hasPeriscope",
            @"name": @"Periscope",
            @"icon": @"periscope-icon",
            @"largeIcon": @"periscope-large",
            @"color": [UIColor colorWithRed: (53.0/255.0) green: (163.0/255.0) blue: (198.0/255.0) alpha: 1.0],
            @"key": @(PeriscopeType),
        },
        @(VineType): @{
            @"property": @"hasVine",
            @"name": @"Vine",
            @"icon": @"vine-icon",
            @"largeIcon": @"vine-large",
            @"color": [UIColor colorWithRed: (0.0/255.0) green: (182.0/255.0) blue: (135.0/255.0) alpha: 1.0],
            @"key": @(VineType),
        },
        @(SoundCloudType): @{
            @"property": @"hasSoundCloud",
            @"name": @"SoundCloud",
            @"icon": @"soundcloud-icon",
            @"largeIcon": @"soundcloud-large",
            @"color": [UIColor colorWithRed: (255.0/255.0) green: (136.0/255.0) blue: (0.0/255.0) alpha: 1.0],
            @"key": @(SoundCloudType),
        },
        @(SinaWeiboType): @{
            @"property": @"hasSinaWeibo",
            @"name": @"Sina Weibo",
            @"icon": @"weibo-icon",
            @"largeIcon": @"weibo-large",
            @"color": [UIColor colorWithRed: (182.0/255.0) green: (48.0/255.0) blue: (47.0/255.0) alpha: 1.0],
            @"key": @(SinaWeiboType),
        },
        @(VKontakteType): @{
            @"property": @"hasVKontakte",
            @"name": @"VKontakte",
            @"icon": @"vk-icon",
            @"largeIcon": @"vk-large",
            @"color": [UIColor colorWithRed: (75.0/255.0) green: (117.0/255.0) blue: (163.0/255.0) alpha: 1.0],
            @"key": @(VKontakteType),
        },
    };
}

@end
