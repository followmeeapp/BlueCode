//
//  BlueSocialNetworksController.m
//  Follow
//
//  Created by Erich Ocean on 7/31/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueSocialNetworksController.h"

#import "BlueApp.h"
#import "BlueClient.h"
#import "BlueModel.h"

#import <NSHash/NSData+NSHash.h>

#import "ServerRequest.h"

#import "BlueNetworkEditController.h"

#import "CardObject.h"
#import "NetworkObject.h"

@interface BlueSocialNetworksController ()

@property (nonatomic, strong) NSArray *array;

@property (nonatomic, copy) NSString *uploadUrl;
@property (nonatomic, copy) NSString *authorizationToken;

@property (nonatomic, copy) NSString *avatarUrl;
@property (nonatomic, copy) NSString *coverPhotoUrl;

@property (nonatomic, assign) BOOL isCreatingCard;

@property (nonatomic, strong) UIPageViewController *pageViewController;

@property (nonatomic, assign) NSInteger pageIndex;

@property (nonatomic, assign) NSTimeInterval started;

@end

@implementation BlueSocialNetworksController

- (IBAction) save: sender
{
    if (self.isCreatingCard) return;

    NSString *fullName = APP_DELEGATE.fullName;
    if (!fullName || [fullName length] == 0) {
        AlertWithMessage(@"You need to at least provide your full name.");
        return;
    }

    self.isCreatingCard = YES;

    UIPageViewController *vc = [[UIPageViewController alloc] initWithTransitionStyle: UIPageViewControllerTransitionStyleScroll
                                                             navigationOrientation:   UIPageViewControllerNavigationOrientationHorizontal
                                                             options:                 nil                                                ];

    vc.dataSource = self;
    vc.delegate = self;

    if (APP_DELEGATE.isUpdatingCard) {
        vc.title = @"Updating Card";

        UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
        UIViewController *vc = [sb instantiateViewControllerWithIdentifier: @"SavingCardToCloud"];

        [self.navigationController pushViewController: vc animated: YES];

    } else {
        vc.title = @"Creating Card";

        UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
        UIViewController *page = [sb instantiateViewControllerWithIdentifier: @"CreateCardTutorial1"];

        self.pageIndex = 0;
        [vc setViewControllers: @[page] direction: UIPageViewControllerNavigationDirectionForward animated: NO completion: nil];

        // TODO(Erich): Any other page view controller configuration?

        [self.navigationController pushViewController: vc animated: YES];
        self.pageViewController = vc;
    }

    // FIXME(Erich): We're not handling server errors!

    NSData *uploadInfo = [NSData dataWithContentsOfURL: [NSURL URLWithString: @"http://199.19.87.92:3004"]];
    NSError *error = nil;
    NSDictionary *dict = [NSJSONSerialization JSONObjectWithData: uploadInfo
                                              options:            0
                                              error:              &error    ];

    if (error) {
        NSLog(@"JSON parse error: %@", error);
        AlertWithMessage(error.localizedDescription);
        return;
    }

    self.uploadUrl = dict[@"uploadUrl"];
    self.authorizationToken = dict[@"authorizationToken"];

    if (APP_DELEGATE.isUpdatingCard) {
        // Applies only to the overall process.
        self.started = [NSDate timeIntervalSinceReferenceDate];
    }

    if (APP_DELEGATE.croppedAvatar) {
        if (!APP_DELEGATE.isUpdatingCard) self.started = [NSDate timeIntervalSinceReferenceDate];
        [self uploadAvatar: UIImageJPEGRepresentation(APP_DELEGATE.croppedAvatar, 0.5)];

    } else if (APP_DELEGATE.isUpdatingCard) {
        [self didUploadAvatarToURL: APP_DELEGATE.card.avatarURLString];

    } else {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [self didUploadAvatarToURL: nil];
        });
    }
}

- (void) uploadAvatar: (NSData *) uploadData
{
    // FIXME(Erich): We're not handling server errors!

    NSString *contentType = @"image/jpeg";      // The content type of the file
    NSString *sha1 = [uploadData SHA1String];   // SHA1 of the file you are uploading
    NSString *fileName = [NSString stringWithFormat: @"%@.jpg", sha1];

    NSURLSession *session = [NSURLSession sharedSession];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL: [NSURL URLWithString: self.uploadUrl]];

    request.HTTPMethod = @"POST";
    [request addValue: self.authorizationToken forHTTPHeaderField: @"Authorization"];
    [request addValue: fileName                forHTTPHeaderField: @"X-Bz-File-Name"];
    [request addValue: contentType             forHTTPHeaderField: @"Content-Type"];
    [request addValue: sha1                    forHTTPHeaderField: @"X-Bz-Content-Sha1"];

    id task = [session uploadTaskWithRequest: request
                       fromData:              uploadData
                       completionHandler:
    ^(NSData *data, NSURLResponse *response, NSError *error) {
        if (data) {
            NSError *error = nil;
            NSDictionary *dict = [NSJSONSerialization JSONObjectWithData: data
                                                      options:            0
                                                      error:              &error];

            if (error) {
                NSLog(@"JSON parse error: %@", error);
                self.isCreatingCard = NO;

            } else {
                NSString *urlPrefix = @"https://f001.backblaze.com/b2api/v1/b2_download_file_by_id?fileId=";
                NSString *fileId = dict[@"fileId"];
                NSString *downloadUrl = [NSString stringWithFormat: @"%@%@", urlPrefix, fileId];

                NSLog(@"downloadUrl = %@", downloadUrl);

                NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
                NSTimeInterval delay = 0.0;

                if (!APP_DELEGATE.isUpdatingCard && now - self.started < 3.0) {
                    delay = 3.0 - (now - self.started);
                }

                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delay * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                    [self didUploadAvatarToURL: downloadUrl];
                });
            }
        }
    }];

    [task resume];
}

- (void) didUploadAvatarToURL: (NSString *) url
{
    self.avatarUrl = url;

    if (!APP_DELEGATE.isUpdatingCard) {
        UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
        UIViewController *page = [sb instantiateViewControllerWithIdentifier: @"CreateCardTutorial2"];

        self.pageIndex = 1;
        [self.pageViewController setViewControllers: @[page] direction: UIPageViewControllerNavigationDirectionForward animated: YES completion: nil];
    }

    if (APP_DELEGATE.croppedCoverPhoto) {
        if (!APP_DELEGATE.isUpdatingCard) self.started = [NSDate timeIntervalSinceReferenceDate];
        [self uploadCoverPhoto: UIImageJPEGRepresentation(APP_DELEGATE.croppedCoverPhoto, 0.5)];

    } else if (APP_DELEGATE.isUpdatingCard) {
        [self didUploadCoverPhotoToURL: APP_DELEGATE.card.backgroundURLString];

    } else {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [self didUploadCoverPhotoToURL: nil];
        });
    }
}

- (void) uploadCoverPhoto: (NSData *) uploadData
{
    // FIXME(Erich): We're not handling server errors!

    NSString *contentType = @"image/jpeg";      // The content type of the file
    NSString *sha1 = [uploadData SHA1String];   // SHA1 of the file you are uploading
    NSString *fileName = [NSString stringWithFormat: @"%@.jpg", sha1];

    NSURLSession *session = [NSURLSession sharedSession];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL: [NSURL URLWithString: self.uploadUrl]];

    request.HTTPMethod = @"POST";
    [request addValue: self.authorizationToken forHTTPHeaderField: @"Authorization"];
    [request addValue: fileName                forHTTPHeaderField: @"X-Bz-File-Name"];
    [request addValue: contentType             forHTTPHeaderField: @"Content-Type"];
    [request addValue: sha1                    forHTTPHeaderField: @"X-Bz-Content-Sha1"];

    id task = [session uploadTaskWithRequest: request
                       fromData:              uploadData
                       completionHandler:
               ^(NSData *data, NSURLResponse *response, NSError *error) {
                   if (data) {
                       NSError *error = nil;
                       NSDictionary *dict = [NSJSONSerialization JSONObjectWithData: data
                                                                 options:            0
                                                                 error:              &error];

                       if (error) {
                           NSLog(@"JSON parse error: %@", error);
                           self.isCreatingCard = NO;

                       } else {
                           NSString *urlPrefix = @"https://f001.backblaze.com/b2api/v1/b2_download_file_by_id?fileId=";
                           NSString *fileId = dict[@"fileId"];
                           NSString *downloadUrl = [NSString stringWithFormat: @"%@%@", urlPrefix, fileId];
                           
                           NSLog(@"downloadUrl = %@", downloadUrl);

                           NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
                           NSTimeInterval delay = 0.0;

                           if (!APP_DELEGATE.isUpdatingCard && now - self.started < 3.0) {
                               delay = 3.0 - (now - self.started);
                           }

                           dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delay * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                               [self didUploadCoverPhotoToURL: downloadUrl];
                           });
                       }
                   }
               }];

    [task resume];
}

- (void) didUploadCoverPhotoToURL: (NSString *) url
{
    self.coverPhotoUrl = url;

    if (!APP_DELEGATE.isUpdatingCard) {
        UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
        UIViewController *page = [sb instantiateViewControllerWithIdentifier: @"CreateCardTutorial3"];

        self.pageIndex = 2;
        [self.pageViewController setViewControllers: @[page] direction: UIPageViewControllerNavigationDirectionForward animated: YES completion: nil];
    }

    NSMutableDictionary *properties = [NSMutableDictionary dictionary];

    properties[@"fullName"] = APP_DELEGATE.fullName;
    properties[@"networks"] = APP_DELEGATE.networks;

    if (APP_DELEGATE.location) properties[@"location"] = APP_DELEGATE.location;
    if (APP_DELEGATE.bio) properties[@"bio"] = APP_DELEGATE.bio;

    if (self.avatarUrl) properties[@"avatarURLString"] = self.avatarUrl;
    if (self.coverPhotoUrl) properties[@"backgroundURLString"] = self.coverPhotoUrl;

    // FIXME(Erich): We're not handling server errors!

    if (APP_DELEGATE.isUpdatingCard) {
        // Now update the actual card.
        CardObject *card = [APP_DELEGATE.blueModel activeUserCard];

        properties[@"id"] = @(card.id);
        properties[@"version"] = @(card.version);

        [APP_DELEGATE.blueClient sendRequest: [ServerRequest newUpdateCardRequestWithId: [APP_DELEGATE.blueClient nextRequestId] properties: properties]];

        [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(didCreateCard:) name: @"DidUpdateCard" object: @(card.id)];

    } else {
        // Now create the actual card and send it.
        self.started = [NSDate timeIntervalSinceReferenceDate];
        [APP_DELEGATE.blueClient sendRequest: [ServerRequest newCreateCardRequestWithId: [APP_DELEGATE.blueClient nextRequestId] properties: properties]];

        [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(didCreateCard:) name: @"DidCreateCard" object: nil];
    }
}

- (void) didCreateCard: (NSNotification *) note
{
    if (APP_DELEGATE.isUpdatingCard) {
        // Wait at least two seconds each time we update our card.
        NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
        NSTimeInterval delay = 0.0;

        if (now - self.started < 2.0) {
            delay = 2.0 - (now - self.started);
        }

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delay * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [self.navigationController dismissViewControllerAnimated: YES completion: nil];
        });

        // FIXME(Erich): Also need to update what is being sent over BLE.

    } else {
        NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
        NSTimeInterval delay = 0.0;

        if (!APP_DELEGATE.isUpdatingCard && now - self.started < 3.0) {
            delay = 3.0 - (now - self.started);
        }

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delay * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
            UIViewController *page = [sb instantiateViewControllerWithIdentifier: @"CreateCardTutorial4"];

            self.pageIndex = 3;
            [self.pageViewController setViewControllers: @[page] direction: UIPageViewControllerNavigationDirectionForward animated: YES completion: nil];
        });
    }
}

- (void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
}

- (void) viewDidLoad
{
    [super viewDidLoad];

    self.array = @[
        @{
            @"name": @"Pokémon Go",
            @"icon": @"pokemongo-tile",
            @"largeIcon": @"pokemon-large",
            @"color": [UIColor colorWithRed: (255.0/255.0) green: (255.0/255.0) blue: (255.0/255.0) alpha: 1.0],
            @"key": @(PokemonGoType),
        },
        @{
            @"name": @"Facebook",
            @"icon": @"facebook-tile",
            @"largeIcon": @"facebook-large",
            @"color": [UIColor colorWithRed: (58.0/255.0) green: (87.0/255.0) blue: (154.0/255.0) alpha: 1.0],
            @"key": @(FacebookType),
        },
        @{
            @"name": @"Twitter",
            @"icon": @"twitter-tile",
            @"largeIcon": @"twitter-large",
            @"color": [UIColor colorWithRed: (80.0/255.0) green: (170.0/255.0) blue: (241.0/255.0) alpha: 1.0],
            @"key": @(TwitterType),
        },
        @{
            @"name": @"Instagram",
            @"icon": @"instagram-tile",
            @"largeIcon": @"instagram-large",
            @"color": [UIColor colorWithRed: (60.0/255.0) green: (113.0/255.0) blue: (157.0/255.0) alpha: 1.0],
            @"key": @(InstagramType),
        },
        @{
            @"name": @"Snapchat",
            @"icon": @"snapchat-tile",
            @"largeIcon": @"snapchat-large",
            @"color": [UIColor colorWithRed: (254.0/255.0) green: (255.0/255.0) blue: (0.0/255.0) alpha: 1.0],
            @"key": @(SnapchatType),
        },
        @{
            @"name": @"Google+",
            @"icon": @"googleplus-tile",
            @"largeIcon": @"googleplus-large",
            @"color": [UIColor colorWithRed: (225.0/255.0) green: (73.0/255.0) blue: (50.0/255.0) alpha: 1.0],
            @"key": @(GooglePlusType),
        },
        @{
            @"name": @"YouTube",
            @"icon": @"youtube-tile",
            @"largeIcon": @"youtube-large",
            @"color": [UIColor colorWithRed: (234.0/255.0) green: (39.0/255.0) blue: (27.0/255.0) alpha: 1.0],
            @"key": @(YouTubeType),
        },
        @{
            @"name": @"Pinterest",
            @"icon": @"pinterest-tile",
            @"largeIcon": @"pinterest-large",
            @"color": [UIColor colorWithRed: (208.0/255.0) green: (26.0/255.0) blue: (31.0/255.0) alpha: 1.0],
            @"key": @(PinterestType),
        },
        @{
            @"name": @"Tumblr",
            @"icon": @"tumblr-tile",
            @"largeIcon": @"tumblr-large",
            @"color": [UIColor colorWithRed: (52.0/255.0) green: (69.0/255.0) blue: (93.0/255.0) alpha: 1.0],
            @"key": @(TumblrType),
        },
        @{
            @"name": @"LinkedIn",
            @"icon": @"linkedin-tile",
            @"largeIcon": @"linkedin-large",
            @"color": [UIColor colorWithRed: (0.0/255.0) green: (116.0/255.0) blue: (182.0/255.0) alpha: 1.0],
            @"key": @(LinkedInType),
        },
        @{
            @"name": @"Periscope",
            @"icon": @"periscope-tile",
            @"largeIcon": @"periscope-large",
            @"color": [UIColor colorWithRed: (53.0/255.0) green: (163.0/255.0) blue: (198.0/255.0) alpha: 1.0],
            @"key": @(PeriscopeType),
        },
        @{
            @"name": @"Vine",
            @"icon": @"vine-tile",
            @"largeIcon": @"vine-large",
            @"color": [UIColor colorWithRed: (0.0/255.0) green: (182.0/255.0) blue: (135.0/255.0) alpha: 1.0],
            @"key": @(VineType),
        },
        @{
            @"name": @"SoundCloud",
            @"icon": @"soundcloud-tile",
            @"largeIcon": @"soundcloud-large",
            @"color": [UIColor colorWithRed: (255.0/255.0) green: (136.0/255.0) blue: (0.0/255.0) alpha: 1.0],
            @"key": @(SoundCloudType),
        },
        @{
            @"name": @"Sina Weibo",
            @"icon": @"weibo-tile",
            @"largeIcon": @"weibo-large",
            @"color": [UIColor colorWithRed: (182.0/255.0) green: (48.0/255.0) blue: (47.0/255.0) alpha: 1.0],
            @"key": @(SinaWeiboType),
        },
        @{
            @"name": @"VKontakte",
            @"icon": @"vk-tile",
            @"largeIcon": @"vk-large",
            @"color": [UIColor colorWithRed: (75.0/255.0) green: (117.0/255.0) blue: (163.0/255.0) alpha: 1.0],
            @"key": @(VKontakteType),
        },
//        @{
//            @"name": @"Habbo",
//            @"icon": @"group-628",
//            @"largeIcon": @"Fill-4",
//            @"color": [UIColor colorWithRed: (58.0/255.0) green: (87.0/255.0) blue: (154.0/255.0) alpha: 1.0],
//            @"key": @(FacebookType),
//        },
//        @{
//            @"name": @"Qzone",
//            @"icon": @"group-626",
//            @"largeIcon": @"Fill-4",
//            @"color": [UIColor colorWithRed: (58.0/255.0) green: (87.0/255.0) blue: (154.0/255.0) alpha: 1.0],
//            @"key": @(FacebookType),
//        },
//        @{
//            @"name": @"Renren",
//            @"icon": @"group-630",
//            @"largeIcon": @"Fill-4",
//            @"color": [UIColor colorWithRed: (58.0/255.0) green: (87.0/255.0) blue: (154.0/255.0) alpha: 1.0],
//            @"key": @(FacebookType),
//        },
    ];
}

- (NSInteger) numberOfSectionsInCollectionView: (UICollectionView *) collectionView
{
    return 1;
}

-(NSInteger)
collectionView:         (UICollectionView *) collectionView
numberOfItemsInSection: (NSInteger)          section
{
    return self.array.count;
}

- (UICollectionViewCell *)
collectionView:         (UICollectionView *) collectionView
cellForItemAtIndexPath: (NSIndexPath *)      indexPath
{
    NSDictionary *props = [self.array objectAtIndex: indexPath.row];

    static NSString *cellIdentifier = @"NetworkCell";

    UICollectionViewCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier: cellIdentifier forIndexPath: indexPath];

//    UILabel *titleLabel = (UILabel *)[cell viewWithTag: 100];
//    titleLabel.text = props[@"name"];
//    titleLabel.textColor = props[@"color"];

    UIImageView *imageView = (UIImageView *)[cell viewWithTag: 101];
    imageView.image = [UIImage imageNamed: props[@"icon"]];

    return cell;
}

- (void)
collectionView:           (UICollectionView *) collectionView
didSelectItemAtIndexPath: (NSIndexPath *)      indexPath
{
    if (indexPath.row == 0) {
        UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
        BlueNetworkEditController *vc = [sb instantiateViewControllerWithIdentifier: @"BluePokemonGoController"];

        [self.navigationController pushViewController: vc animated: YES];

    } else {
        NSDictionary *props = [self.array objectAtIndex: indexPath.row];

        UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
        BlueNetworkEditController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueNetworkEditController"];

        vc.networkKey = props[@"key"];
        vc.networkName = props[@"name"];
        vc.promptText = [NSString stringWithFormat: @"%@ Username", props[@"name"]];
        vc.backgroundColor = props[@"color"];
        vc.image = [UIImage imageNamed: props[@"largeIcon"]];

        [self.navigationController pushViewController: vc animated: YES];
    }
}

#pragma mark UIPageViewControllerDataSource

//- (UIViewController *)
//pageViewController:                 (UIPageViewController *) pageViewController
//viewControllerBeforeViewController: (UIViewController *)     viewController
//{
//}

//- (UIViewController *)
//pageViewController:                (UIPageViewController *) pageViewController
//viewControllerAfterViewController: (UIViewController *)     viewController
//{
//}

- (NSInteger) presentationCountForPageViewController: (UIPageViewController *) pageViewController
{
    return 4;
}

- (NSInteger) presentationIndexForPageViewController: (UIPageViewController *) pageViewController
{
    return self.pageIndex;
}

#pragma mark UIPageViewControllerDelegate

//- (void)
//pageViewController:              (UIPageViewController *)        pageViewController
//willTransitionToViewControllers: (NSArray<UIViewController *> *) pendingViewControllers
//{
//}

//- (void)
//pageViewController:      (UIPageViewController *)        pageViewController
//didFinishAnimating:      (BOOL)                          finished
//previousViewControllers: (NSArray<UIViewController *> *) previousViewControllers
//transitionCompleted:     (BOOL)                          completed
//{
//}

@end
