//
//  BlueMainController.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueMainController.h"

#import <MessageUI/MessageUI.h>

#import <Crashlytics/Crashlytics.h>
#import <DigitsKit/DigitsKit.h>
#import <Realm/Realm.h>
#import <SDWebImage/UIImageView+WebCache.h>
#import <CCMPopup/CCMPopupTransitioning.h>
#import <CCMPopup/CCMPopupSegue.h>
#import <REMenu/REMenu.h>

#import "BlueApp.h"
#import "BlueModel.h"
#import "BlueClient.h"
#import "BlueBackup.h"

#import "ServerRequest.h"

#import "BlueRegisterController.h"
#import "BlueCardController.h"
#import "BlueCardPageController.h"
#import "BlueIntroController.h"

#import "UserObject.h"
#import "DeviceObject.h"
#import "SectionObject.h"
#import "CardObject.h"

#import "BlueJoinCell.h"
#import "BlueCreateCardCell.h"
#import "BlueYourCardCell.h"
#import "BlueCardCell.h"
#import "BlueBluetoothCardCell.h"

static NSString *JOIN_CELL           = @"JoinCell";
static NSString *CREATE_CARD_CELL    = @"CreateCardCell";
static NSString *YOUR_CARD_CELL      = @"YourCardCell";
static NSString *CARD_CELL           = @"CardCell";
static NSString *BLUETOOTH_CARD_CELL = @"BluetoothCardCell";

typedef NS_ENUM(NSInteger, MainStatus) {
    Unknown = 0,
    NeedToJoin,
    NeedToCreateCard,
    HaveCard,
};

@interface BlueMainController () <MFMailComposeViewControllerDelegate>

@property (nonatomic, assign) MainStatus mainStatus;

@property (nonatomic, strong) RLMNotificationToken *notification;

@property (nonatomic, strong) NSIndexPath *currentCard;
@property (nonatomic, strong) NSIndexPath *pendingCard;

@property (nonatomic, strong) NSArray *networkKeys;
@property (nonatomic, strong) NSDictionary *networkInfo;

@property (nonatomic, strong) REMenu *menu;

@property (nonatomic, strong) MFMailComposeViewController *mailController;

@property (nonatomic, strong) DeviceObject *activeDevice;
@property (nonatomic, assign) NSInteger visibleCardCount;

@property (nonatomic, assign) BOOL suppressTableViewUpdateOnDeletion;

@end

@implementation BlueMainController

- (void) configureMenu
{
    REMenuItem *policiesItem = [[REMenuItem alloc] initWithTitle:    @"Policies"
                                                   subtitle:         nil
                                                   image:            nil
                                                   highlightedImage: nil
                                                   action:
    ^(REMenuItem *item) {
        UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
        UIViewController *vc = [sb instantiateViewControllerWithIdentifier: @"TermsAndConditions"];
        [self.navigationController pushViewController: vc animated: YES];
    }];

    REMenuItem *feedbackItem = [[REMenuItem alloc] initWithTitle:    @"Feedback"
                                                   image:            nil
                                                   highlightedImage: nil
                                                   action:
    ^(REMenuItem *item) {
        NSString *emailTitle = @"Blue Feedback";
        NSString *messageBody = @"";

        MFMailComposeViewController *mc = [[MFMailComposeViewController alloc] init];
        if (!mc) return;

        mc.mailComposeDelegate = self;
        [mc setToRecipients: @[@"feedback@blue.social"]];
        [mc setSubject: emailTitle];
        [mc setMessageBody: messageBody isHTML: NO];

        [self presentViewController: mc animated: YES completion: NULL];
    }];

    REMenuItem *backupItem = [[REMenuItem alloc] initWithTitle:    @"Backup"
                                                 subtitle:         nil
                                                 image:            nil
                                                 highlightedImage: nil
                                                 action:
    ^(REMenuItem *item) {
        NSLog(@"Triggering backup.");
        [APP_DELEGATE.blueBackup backupNow: nil];
    }];

    REMenuItem *logoutItem = [[REMenuItem alloc] initWithTitle:    @"Logout"
                                                 subtitle:         nil
                                                 image:            nil
                                                 highlightedImage: nil
                                                 action:
    ^(REMenuItem *item) {
        [APP_DELEGATE.blueBackup logout: nil];
    }];

    policiesItem.tag = 1;
    feedbackItem.tag = 2;
    backupItem.tag = 3;
    logoutItem.tag = 4;

    self.menu = [[REMenu alloc] initWithItems: @[policiesItem, feedbackItem, backupItem, logoutItem]];

    // Blurred background in iOS 7
    self.menu.liveBlur = YES;
    self.menu.liveBlurBackgroundStyle = REMenuLiveBackgroundStyleDark;

    UIImage *image = [UIImage imageNamed: @"hamburger-icon"];
    UIBarButtonItem *menuButton = [[UIBarButtonItem alloc] initWithImage: image
                                                           style:         UIBarButtonItemStylePlain
                                                           target:        self
                                                           action:        @selector(toggleMenu:)   ];
    self.navigationItem.leftBarButtonItem = menuButton;
}

- (IBAction) toggleMenu: sender
{
    if (self.menu.isOpen) {
        return [self.menu close];
    }

    [self.menu showFromNavigationController: self.navigationController];
}

- (void)
mailComposeController: (MFMailComposeViewController *) controller
didFinishWithResult:   (MFMailComposeResult)           result
error:                 (NSError *)                     error
{
    [self dismissViewControllerAnimated: YES completion: NULL];
}

- (void) configureTour
{
    UIImage *image = [UIImage imageNamed: @"tour-icon"];
    UIBarButtonItem *menuButton = [[UIBarButtonItem alloc] initWithImage: image
                                                           style:         UIBarButtonItemStylePlain
                                                           target:        self
                                                           action:        @selector(openTour:)     ];
    self.navigationItem.rightBarButtonItem = menuButton;
}

- (void) setupNetworkInfo
{
    self.networkKeys = @[
        @(InstagramType),
        @(SnapchatType),
        @(TwitterType),
        @(FacebookType),
        @(SoundCloudType),
        @(YouTubeType),
        @(LinkedInType),
//        @(VineType),
        @(GooglePlusType),
        @(PinterestType),
        @(PeriscopeType),
        @(TumblrType),
        @(SinaWeiboType),
        @(VKontakteType),
    ];

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
//        @(VineType): @{
//            @"property": @"hasVine",
//            @"name": @"Vine",
//            @"icon": @"vine-icon",
//            @"largeIcon": @"vine-large",
//            @"color": [UIColor colorWithRed: (0.0/255.0) green: (182.0/255.0) blue: (135.0/255.0) alpha: 1.0],
//            @"key": @(VineType),
//        },
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

- (void) updateStatus
{
    MainStatus old = _mainStatus;

    UserObject *user = [APP_DELEGATE.blueModel activeUser];
    if (user) {
        if (user.cardId > 0) {
            _mainStatus = HaveCard;

        } else {
            _mainStatus = NeedToCreateCard;
        }

    } else {
        _mainStatus = NeedToJoin;
    }

    if (old != _mainStatus) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [self.tableView reloadSections:   [NSIndexSet indexSetWithIndex: 0]
                            withRowAnimation: UITableViewRowAnimationNone     ];
        });
    }
}

- (void) refreshUserCard
{
    [self.tableView reloadSections:   [NSIndexSet indexSetWithIndex: 0]
                    withRowAnimation: UITableViewRowAnimationNone     ];
}

- (void) didCreateUser: (NSNotification *) note
{
    [self updateStatus];
}

- (void) didCreateCard: (NSNotification *) note
{
    [self updateStatus];

    // If we don't have our card's images saved locally, we need to download them now.
    CardObject *card = [APP_DELEGATE.blueModel activeUserCard];
    if (!card) return;

    if (card.avatarURLString) {
        NSURL *url = [NSURL URLWithString: card.avatarURLString];
        [self downloadAvatar: url];
    }

    if (card.backgroundURLString) {
        NSURL *url = [NSURL URLWithString: card.backgroundURLString];
        [self downloadCoverPhoto: url];
    }
}

- (void) downloadAvatar: (NSURL *) url
{
    NSURLSession *session = [NSURLSession sharedSession];

    id task = [session downloadTaskWithURL: url
                       completionHandler:
    ^(NSURL *location, NSURLResponse *response, NSError *error) {
        NSData *data = location ? [NSData dataWithContentsOfURL: location] : nil;

        if (data) {
            [data writeToURL: APP_DELEGATE.localAvatarURL atomically: YES];
            [data writeToURL: APP_DELEGATE.localOriginalAvatarURL atomically: YES];
            [data writeToURL: APP_DELEGATE.localCroppedAvatarURL atomically: YES];

            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                [self refreshUserCard];
            });
        }
    }];

    [task resume];
}

- (void) downloadCoverPhoto: (NSURL *) url
{
    NSURLSession *session = [NSURLSession sharedSession];

    id task = [session downloadTaskWithURL: url
                       completionHandler:
    ^(NSURL *location, NSURLResponse *response, NSError *error) {
        NSData *data = location ? [NSData dataWithContentsOfURL: location] : nil;

        if (data) {
            [data writeToURL: APP_DELEGATE.localCoverPhotoURL atomically: YES];
            [data writeToURL: APP_DELEGATE.localOriginalCoverPhotoURL atomically: YES];
            [data writeToURL: APP_DELEGATE.localCroppedCoverPhotoURL atomically: YES];

            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                [self refreshUserCard];
            });
        }
    }];

    [task resume];
}

- (void) viewDidLoad
{
    [super viewDidLoad];

//    [[Digits sharedInstance] logOut];
    [self configureMenu];
    [self configureTour];
    [self setupNetworkInfo];
    [self updateStatus];

    UIRefreshControl *refreshControl = [[UIRefreshControl alloc] init];
    [refreshControl addTarget: self action: @selector(refresh:) forControlEvents: UIControlEventValueChanged];
    [self setRefreshControl: refreshControl];

    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(beginRefreshingTableView:) name: @"RefreshCards" object: nil];

//    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(moveCardToFront:) name: @"MoveCardToFront" object: nil];

    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(didCreateUser:) name: @"DidCreateUser" object: nil];
    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(didCreateCard:) name: @"DidCreateCard" object: nil];
    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(didUpdateCardsFromBackup:) name: @"BlueDidUpdateCardsFromBackup" object: nil];

    self.clearsSelectionOnViewWillAppear = YES;

    UIImage *image = [UIImage imageNamed: @"navbar-icon"];
    UIButton *titleButton = [UIButton buttonWithType: UIButtonTypeCustom];
    [titleButton setImage: image forState: UIControlStateNormal];
    [titleButton setUserInteractionEnabled: YES];
    [titleButton addTarget: self action: @selector(editUserCard:) forControlEvents: UIControlEventTouchUpInside];
    [titleButton setFrame: CGRectMake(0, 0, 116, 53.0)];
    self.navigationItem.titleView = titleButton;

    UIColor *tintColor = APP_DELEGATE.black;
    [self.navigationController.navigationBar setTintColor: tintColor];
    [self.navigationController.navigationBar setTitleTextAttributes: @{ NSForegroundColorAttributeName: tintColor }];

    // Remove 1px high first "section"
    self.tableView.contentInset = UIEdgeInsetsMake(-1.0f, 0.0f, 0.0f, 0.0);

    RLMRealm *realm = [RLMRealm defaultRealm];

    [realm beginWriteTransaction];

    self.activeDevice = [APP_DELEGATE.blueModel activeDevice];
    self.visibleCardCount = self.activeDevice.visibleCards.count;

    if (self.visibleCardCount < 3) {
        UIImageView *imageView = [[UIImageView alloc] initWithImage: [UIImage imageNamed: @"discovery-icon"]];
        [imageView setContentMode: UIViewContentModeBottom];
        self.tableView.backgroundView = imageView;
    }

    if (self.activeDevice.needsToBeCreated) {
        [realm addOrUpdateObject: self.activeDevice];
    }

    [realm commitWriteTransaction];

    __weak typeof(self) weakSelf = self;
    self.notification = [[RLMRealm defaultRealm] addNotificationBlock: ^(RLMNotification notification, RLMRealm *realm) {
        if (self.suppressTableViewUpdateOnDeletion) {
            self.suppressTableViewUpdateOnDeletion = NO;
            return;
        }

        UITableView *tv = weakSelf.tableView;
        [tv beginUpdates];

        // Reload the User's own card.
        [tv reloadRowsAtIndexPaths: @[[NSIndexPath indexPathForRow: 0 inSection: 0]] withRowAnimation: UITableViewRowAnimationAutomatic];

        // Reload recents.
        // FIXME(Erich): This assumes visibleCards is in reverse order from how it's actually stored,
        // which we arrange for transparently in DeviceObject. It'd be MUCH better if instead we could
        // work with visibleCards in its native order, as it takes time and energy to constantly be
        // reversing the visibleCards array every time we add a new card.
        NSInteger old = weakSelf.visibleCardCount;
        DeviceObject *activeDevice = [APP_DELEGATE.blueModel activeDevice];
        NSAssert(!activeDevice.needsToBeCreated, @"Active device should already be created!");

        NSArray *visibleCards = activeDevice.visibleCards;

        // Gather rows to insert.
        NSMutableArray *insert = [NSMutableArray arrayWithCapacity: visibleCards.count - old];
        for (NSInteger idx=0, len=visibleCards.count - old; idx<len; ++idx) {
            [insert addObject: [NSIndexPath indexPathForRow: idx inSection: 1]];
        }
        [tv insertRowsAtIndexPaths: insert withRowAnimation: UITableViewRowAnimationAutomatic];

        // Gather rows to update.
        NSMutableArray *update = [NSMutableArray arrayWithCapacity: visibleCards.count - insert.count];
        for (NSInteger idx=visibleCards.count - old, len=old; idx<len; ++idx) {
            [update addObject: [NSIndexPath indexPathForRow: idx inSection: 1]];
        }
        [tv reloadRowsAtIndexPaths: update withRowAnimation: UITableViewRowAnimationAutomatic];

        weakSelf.activeDevice = activeDevice;
        weakSelf.visibleCardCount = visibleCards.count;

        [tv endUpdates];
    }];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        // If we have no cards, add the CEO card.
        RLMRealm *realm = [RLMRealm defaultRealm];

        [realm beginWriteTransaction];

        RLMResults *results = [CardObject allObjects];
        if (results.count == 0) {
            CardObject *ceoCard = [[CardObject alloc] init];

            ceoCard.id = 0;
            ceoCard.version = 1; // Prevent it from being treated as not-fully-loaded.
            ceoCard.fullName = @"Blue";
            ceoCard.location = @"Riverside, CA";
            ceoCard.bio = @"The New Way to Network\n\nShare your Social Media with ANY iPhone user using Bluetooth Low Energy even if they do NOT have Blue\n\n\n\n\nhttp://www.Blue.Social";

            NSString *avatarFilePath = [[NSBundle mainBundle] pathForResource: @"jose-avatar" ofType: @"jpg"];
            NSURL *avatarImageURL = [NSURL fileURLWithPath: avatarFilePath];
            ceoCard.avatarURLString = [avatarImageURL absoluteString];

            NSString *coverPhotoFilePath = [[NSBundle mainBundle] pathForResource: @"jose-cover-photo" ofType: @"jpg"];
            NSURL *coverPhotoImageURL = [NSURL fileURLWithPath: coverPhotoFilePath];
            ceoCard.backgroundURLString = [coverPhotoImageURL absoluteString];

            NetworkObject *instagram = [[NetworkObject alloc] init];
            instagram.guid = [[NSUUID UUID] UUIDString];
            instagram.cardId = 0;
            instagram.type = InstagramType;
            instagram.username = @"Blue_Social";
            ceoCard.hasInstagram = YES;
            [ceoCard.networks addObject: instagram];

            NetworkObject *twitter = [[NetworkObject alloc] init];
            twitter.guid = [[NSUUID UUID] UUIDString];
            twitter.cardId = 0;
            twitter.type = TwitterType;
            twitter.username = @"BLUESocialApp";
            ceoCard.hasTwitter = YES;
            [ceoCard.networks addObject: twitter];

            NetworkObject *googlePlus = [[NetworkObject alloc] init];
            googlePlus.guid = [[NSUUID UUID] UUIDString];
            googlePlus.cardId = 0;
            googlePlus.type = GooglePlusType;
            googlePlus.username = @"106740032818432453647";
            ceoCard.hasGooglePlus = YES;
            [ceoCard.networks addObject: googlePlus];

            NetworkObject *facebook = [[NetworkObject alloc] init];
            facebook.guid = [[NSUUID UUID] UUIDString];
            facebook.cardId = 0;
            facebook.type = FacebookType;
            facebook.username = @"BLUESocialApp";
            ceoCard.hasFacebook = YES;
            [ceoCard.networks addObject: facebook];

            NetworkObject *pinterest = [[NetworkObject alloc] init];
            pinterest.guid = [[NSUUID UUID] UUIDString];
            pinterest.cardId = 0;
            pinterest.type = PinterestType;
            pinterest.username = @"BlueSocialApp";
            ceoCard.hasPinterest = YES;
            [ceoCard.networks addObject: pinterest];

            NetworkObject *youTube = [[NetworkObject alloc] init];
            youTube.guid = [[NSUUID UUID] UUIDString];
            youTube.cardId = 0;
            youTube.type = YouTubeType;
            youTube.username = @"UC6VjehRGZi9-Qoy3F7UVjyw";
            ceoCard.hasYouTube = YES;
            [ceoCard.networks addObject: youTube];

            [realm addOrUpdateObject: instagram];
            [realm addOrUpdateObject: twitter];
            [realm addOrUpdateObject: googlePlus];
            [realm addOrUpdateObject: facebook];
            [realm addOrUpdateObject: pinterest];
            [realm addOrUpdateObject: youTube];

            [realm addOrUpdateObject: ceoCard];

            DeviceObject *activeDevice = [APP_DELEGATE.blueModel activeDevice];

            [activeDevice updateVisibleCards: @[@0] hiddenCards: @[]];

            [realm addOrUpdateObject: activeDevice];

            self.activeDevice = activeDevice;
            self.visibleCardCount = 1;

            self.suppressTableViewUpdateOnDeletion = YES; // HACK
            UITableView *tv = self.tableView;
            [tv beginUpdates];
            [tv insertRowsAtIndexPaths: @[[NSIndexPath indexPathForRow: 0 inSection: 1]] withRowAnimation: UITableViewRowAnimationAutomatic];
            [tv endUpdates];
       }

        [realm commitWriteTransaction];
    });
}

- (void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
}

- (void) didUpdateCardsFromBackup: (NSNotification *) note
{
    self.activeDevice = [APP_DELEGATE.blueModel activeDevice];
    NSAssert(!self.activeDevice.needsToBeCreated, @"Active device should already be created!");

    self.visibleCardCount = self.activeDevice.visibleCards.count;

    [self.tableView reloadData];
}

// TODO(Erich): Test this!
//- (void) moveCardToFront: (NSNotification *) note
//{
//    NSNumber *cardId = note.userInfo[@"cardId"];
//
//    RLMRealm *realm = [RLMRealm defaultRealm];
//    [realm beginWriteTransaction];
//
//    DeviceObject *activeDevice = [APP_DELEGATE.blueModel activeDevice];
//    NSArray *visibleCards = activeDevice.visibleCards;
//
//    // Find the index of the card we want to move.
//    NSInteger idx = [visibleCards indexOfObject: cardId];
//
//    if (idx == 0) {
//        [realm commitWriteTransaction];
//        return;
//    }
//
//    // Update the section array to match.
//    // TODO(Erich): Make this happen.
//    NSMutableArray *mary = [[[visibleCards reverseObjectEnumerator] allObjects] mutableCopy];
//    [mary removeObject: cardId];
//    [mary addObject: cardId];
//
//    [activeDevice updateVisibleCards: mary hiddenCards: @[]];
//
//    [realm addOrUpdateObject: activeDevice];
//    [realm commitWriteTransaction];
//
//    // Update our internal properties.
//    self.activeDevice = activeDevice;
//    self.visibleCardCount = visibleCards.count; // Unchanged
//
//    // Update the table view.
//    UITableView *tv = self.tableView;
//    [tv beginUpdates];
//
//    [tv insertRowsAtIndexPaths: @[[NSIndexPath indexPathForRow: 0   inSection: 1]] withRowAnimation: UITableViewRowAnimationAutomatic];
//    [tv deleteRowsAtIndexPaths: @[[NSIndexPath indexPathForRow: idx inSection: 1]] withRowAnimation: UITableViewRowAnimationAutomatic];
//
//    [tv endUpdates];
//
//    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
//        [tv beginUpdates];
//
//        [tv reloadRowsAtIndexPaths: @[[NSIndexPath indexPathForRow: 1 inSection: 1]] withRowAnimation: UITableViewRowAnimationNone];
//
//        [tv endUpdates];
//    });
//}

- (void) beginRefreshingTableView: (NSNotification *) note;
{
    // Check if contentOffset is zero. http://stackoverflow.com/a/16250793
    if (fabsf((float)self.tableView.contentOffset.y) < FLT_EPSILON) {
        [UIView animateWithDuration: 0.25 delay: 0 options: UIViewAnimationOptionBeginFromCurrentState animations: ^{
            self.tableView.contentOffset = CGPointMake(0, -self.refreshControl.frame.size.height);

        } completion: ^(BOOL finished) {
            [self.refreshControl beginRefreshing];
        }];
    }
}

// TODO(Erich): Make this do an active Bluetooth scan.
- (void) refresh: (id) sender
{
//    NSLog(@"Begin refreshing");
//
//    BOOL didReferesh = NO;
//
//    RLMResults *array = self.array;
//    BlueClient *client = APP_DELEGATE.blueClient;
//
//    for (NSInteger idx=0, len=array.count; idx<len; ++idx) {
//        CardObject *card = array[idx];
//        if (card.version <= 0) {
//            didReferesh = YES;
//            NSData *cardRequestPacket = [ServerRequest newCardRequestWithId: [client nextRequestId] cardId: card.id version: 0];
//            [client sendRequest: cardRequestPacket];
//            NSLog(@"refreshing card = %@ with version = %@", @(card.id), @(card.version));
//        }
//    }
//
//    if (!didReferesh) {
//        NSLog(@"Nothing to refresh");
//        [(UIRefreshControl *)sender endRefreshing];
//
//    } else {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1.5 * NSEC_PER_SEC), dispatch_get_main_queue(), ^(void) {
//            NSLog(@"End Refreshing");
            [self.tableView reloadData];
            [(UIRefreshControl *)sender endRefreshing];
        });
//    }
}

#pragma mark - Actions

- (IBAction) openTour: sender
{
    BlueIntroController *vc = [[BlueIntroController alloc] init];

    vc.isUserInitiatedTour = YES;

    [self presentViewController: vc animated: YES completion:^{

    }];
}

- (IBAction) editUserCard: sender
{
    CardObject *userCard = [APP_DELEGATE.blueModel activeUserCard];
    if (!userCard) return;

    [self tableView: self.tableView willSelectRowAtIndexPath: [NSIndexPath indexPathForRow: 0 inSection: 0]];
}

#pragma mark - Table view delegate

- (NSIndexPath *)
tableView:                (UITableView *) tableView
willSelectRowAtIndexPath: (NSIndexPath *) indexPath
{
    if (indexPath.section == 0) {
        if (_mainStatus == NeedToJoin) {
            UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
            BlueRegisterController *completionViewController = [sb instantiateViewControllerWithIdentifier: @"BlueRegisterController"];

            DGTAppearance *appearance = [[DGTAppearance alloc] init];
            appearance.backgroundColor = APP_DELEGATE.blue1;
            appearance.accentColor = [UIColor whiteColor];

            appearance.headerFont = [UIFont systemFontOfSize: 18];
            appearance.labelFont  = [UIFont systemFontOfSize: 16];
            appearance.bodyFont   = [UIFont systemFontOfSize: 16];

            DGTAuthenticationConfiguration *configuration = [[DGTAuthenticationConfiguration alloc] initWithAccountFields: DGTAccountFieldsEmail];
            configuration.appearance = appearance;
            configuration.title = @"Join Blue";

            [[Digits sharedInstance] authenticateWithNavigationViewController: self.navigationController
                                     configuration:                            configuration
                                     completionViewController:                 completionViewController];

            return nil;

        } else {
            UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
            BlueCardController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueCardController"];
            vc.isEditableCard = YES;

            CardObject *userCard = [APP_DELEGATE.blueModel activeUserCard];
            if (userCard) {
                [APP_DELEGATE.blueAnalytics viewOwnCard: userCard.id];

                vc.card = userCard;
                vc.isUserCard = YES;
                vc.shouldEditUserCard = NO;

            } else {
                [APP_DELEGATE.blueAnalytics createCard];

                [APP_DELEGATE prepareToEditUserCard];
                vc.shouldEditUserCard = YES;
            }

            [self.navigationController pushViewController: vc animated: YES];
            return nil;
        }
    }

    return indexPath;
}

#pragma mark - Table view data source

- (NSInteger) numberOfSectionsInTableView: (UITableView *) tableView
{
    return 2;
}

- (NSInteger)
tableView:             (UITableView *) tableView
numberOfRowsInSection: (NSInteger)     section
{
    if (section == 0) {
        return 1;

    } else if (section == 1) {
        return self.visibleCardCount;

    } else {
        return 0;
    }
}

- (void)
configureBlueCardCell: (BlueCardCell *) cell
withCardObject:        (CardObject *)   card
isUserCard:            (BOOL)           isUserCard
{
    if (card && card.version <= 0) {
        NSData *cardRequestPacket = [ServerRequest newCardRequestWithId: [APP_DELEGATE.blueClient nextRequestId] cardId: card.id version: card.version];
        [APP_DELEGATE.blueClient sendRequest: cardRequestPacket];
    }

    cell.avatar.clipsToBounds = YES;
    cell.avatar.layer.cornerRadius = 38; // this value vary as per your desire
    cell.avatar.layer.borderColor = [UIColor whiteColor].CGColor;
    cell.avatar.layer.borderWidth = 1.0;

    if (isUserCard && !card) {
        cell.fullName.text = @"Your Name";
        cell.location.text = @"Tap to configure your own card.";
        cell.coverPhoto.image = [UIImage imageNamed: @"default-card-background"];
        cell.avatar.image = [UIImage imageNamed: @"missing-avatar"];

        cell.networks = @[];
        [cell.collectionView reloadData];

    } else {
        cell.fullName.text = card.fullName;
        cell.location.text = card.location;

        if (isUserCard) {
            if (card.backgroundURLString) {
                const char *filePath = [APP_DELEGATE.localCoverPhotoURL fileSystemRepresentation];
                cell.coverPhoto.image = [UIImage imageWithContentsOfFile: [NSString stringWithUTF8String: filePath]];

            } else {
                cell.coverPhoto.image = [UIImage imageNamed: @"default-card-background"];
            }

        } else if (card.backgroundURLString) {
            [cell.coverPhoto sd_setImageWithURL: [NSURL URLWithString: card.backgroundURLString]
                             placeholderImage:   [UIImage imageNamed: @"default-card-background"]];

        } else {
            cell.coverPhoto.image = [UIImage imageNamed: @"default-card-background"];
        }

        if (isUserCard) {
            if (card.avatarURLString) {
                const char *filePath = [APP_DELEGATE.localAvatarURL fileSystemRepresentation];
                cell.avatar.image = [UIImage imageWithContentsOfFile: [NSString stringWithUTF8String: filePath]];

            } else {
                cell.avatar.image = [UIImage imageNamed: @"missing-avatar"];
            }

        } else if (card.avatarURLString) {
            [cell.avatar sd_setImageWithURL: [NSURL URLWithString: card.avatarURLString]
                         placeholderImage:   [UIImage imageNamed: @"missing-avatar"]   ];

        } else {
            cell.avatar.image = [UIImage imageNamed: @"missing-avatar"];
        }

        // Set the social network icons correctly.
        __block NSInteger index = 0;
        NSMutableArray *networks = [NSMutableArray array];
        [self.networkKeys enumerateObjectsUsingBlock: ^(NSNumber *networkType, NSUInteger idx, BOOL *stop) {
            NSDictionary *networkInfo = self.networkInfo[networkType];

            if ([[card valueForKey: networkInfo[@"property"]] boolValue]) {
                [networks addObject: networkType];
            }

            if (index >= 5) *stop = YES;
        }];

        cell.networks = networks;
        [cell.collectionView reloadData];
    }
}

- (UITableViewCell *)
tableView:             (UITableView *) tableView
cellForRowAtIndexPath: (NSIndexPath *) indexPath
{
    if (indexPath.section == 0) {
        CardObject *card = [APP_DELEGATE.blueModel activeUserCard];
        BlueCardCell *cell = [tableView dequeueReusableCellWithIdentifier: CARD_CELL forIndexPath: indexPath];

        [self configureBlueCardCell: cell withCardObject: card isUserCard: YES];

        return cell;

    } else if (indexPath.section == 1) {
        NSNumber *cardId = self.activeDevice.visibleCards[indexPath.row];
        CardObject *card = [CardObject objectForPrimaryKey: cardId];
        BlueCardCell *cell = [tableView dequeueReusableCellWithIdentifier: CARD_CELL forIndexPath: indexPath];

        [self configureBlueCardCell: cell withCardObject: card isUserCard: NO];

        return cell;
    }

    return nil; // Shouldn't happen!
}

- (CGFloat)
tableView:                (UITableView *) tableView
heightForHeaderInSection: (NSInteger)     section
{
    if (section == 0) return 1.0f;
    else return 36.0f;
}

- (NSString *)
tableView:               (UITableView *) tableView
titleForHeaderInSection: (NSInteger)     section
{
    if (section == 0) return nil;
    else return @"Who's here";
}

- (CGFloat)
tableView:               (UITableView *) tableView
heightForRowAtIndexPath: (NSIndexPath *) indexPath
{
    return 121;
}

- (void)
tableView:             (UITableView *) tableView
willDisplayHeaderView: (UIView *)      view
forSection:            (NSInteger)     section
{
    UITableViewHeaderFooterView *header = (UITableViewHeaderFooterView *)view;

    header.tintColor = APP_DELEGATE.grey; // Set the background color
    header.textLabel.textColor = APP_DELEGATE.black;
    header.textLabel.font = [UIFont boldSystemFontOfSize: 16];

    CGRect headerFrame = header.frame;
    header.textLabel.frame = headerFrame;
    header.textLabel.textAlignment = NSTextAlignmentLeft;
}

- (BOOL)
tableView:             (UITableView *) tableView
canEditRowAtIndexPath: (NSIndexPath *) indexPath
{
    if (indexPath.section == 0) return NO;
    else if ([APP_DELEGATE.blueModel activeUserCard] == nil) return NO;
    else return YES;
}

- (void)
tableView:          (UITableView *)               tableView
commitEditingStyle: (UITableViewCellEditingStyle) editingStyle
forRowAtIndexPath:  (NSIndexPath *)               indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        self.suppressTableViewUpdateOnDeletion = YES;

        RLMRealm *realm = [RLMRealm defaultRealm];

        [realm beginWriteTransaction];

        NSNumber *cardId = self.activeDevice.visibleCards[indexPath.row];
        CardObject *card = [CardObject objectForPrimaryKey: cardId];

        [realm deleteObjects: card.networks];
        [realm deleteObject: card];

        // Update activeDevice
        DeviceObject *activeDevice = [APP_DELEGATE.blueModel activeDevice];
        NSMutableArray *ary = [activeDevice.visibleCards mutableCopy];
        [ary removeObjectAtIndex: indexPath.row];

        [activeDevice updateVisibleCards: [[ary reverseObjectEnumerator] allObjects] hiddenCards: @[]];
        self.activeDevice = activeDevice;
        self.visibleCardCount = activeDevice.visibleCards.count;

        [realm commitWriteTransaction];

        // Also delete the row from the data source
        [tableView deleteRowsAtIndexPaths: @[indexPath] withRowAnimation: UITableViewRowAnimationFade];
    }
}

- (void)
tableView:               (UITableView *) tableView
didSelectRowAtIndexPath: (NSIndexPath *) indexPath
{
    CardObject *card;

    if (indexPath.section == 0) {
        card = [APP_DELEGATE.blueModel activeUserCard];

    } else {
        NSNumber *cardId = self.activeDevice.visibleCards[indexPath.row];
        card = [CardObject objectForPrimaryKey: cardId];
    }

    if (card.version == 0) {
        // This is a Bluetooth-only card.
        if (APP_DELEGATE.blueClient.hasWebSocket) {
            NSData *cardRequestPacket = [ServerRequest newCardRequestWithId: [APP_DELEGATE.blueClient nextRequestId] cardId: card.id version: 0];
            [APP_DELEGATE.blueClient sendRequest: cardRequestPacket];

        } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"No Internet?"
                                                          message:           @"We need to fetch the full card info from the Blue cloud. We'll do that once you have Internet access again."
                                                          delegate:          nil
                                                          cancelButtonTitle: @"OK"
                                                          otherButtonTitles: nil                                                                                                          ];
            [alertView show];
            return;
#pragma clang diagnostic pop
        }
    }

    self.currentCard = indexPath;

    [APP_DELEGATE.blueAnalytics viewCardByTappingRow: card.id];

    UIPageViewController *vc = [[BlueCardPageController alloc] initWithTransitionStyle: UIPageViewControllerTransitionStyleScroll
                                                               navigationOrientation:   UIPageViewControllerNavigationOrientationHorizontal
                                                               options:                 nil                                                ];

    vc.dataSource = self;
    vc.delegate = self;

    // TODO(Erich): Any other page view controller configuration?
    UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
    BlueCardController *cc = [sb instantiateViewControllerWithIdentifier: @"BlueCardController"];

    cc.card = card;
    cc.indexPath = indexPath;

    [vc setViewControllers: @[cc] direction: UIPageViewControllerNavigationDirectionForward animated: NO completion: nil];

    UINavigationController *nc = [[UINavigationController alloc] initWithRootViewController: vc];

    CCMPopupTransitioning *popup = [CCMPopupTransitioning sharedInstance];
    popup.dismissableByTouchingBackground = YES;
    popup.destinationBounds = [[UIScreen mainScreen] bounds];
    popup.presentedController = nc;
    popup.presentingController = self;

    [self presentViewController: nc animated: YES completion: nil];
}

// Override to support rearranging the table view.
//- (void)
//tableView:          (UITableView *) tableView
//moveRowAtIndexPath: (NSIndexPath *) fromIndexPath
//toIndexPath:        (NSIndexPath *) toIndexPath
//{
//}

// Override to support conditional rearranging of the table view.
//- (BOOL)
//tableView:             (UITableView *) tableView
//canMoveRowAtIndexPath: (NSIndexPath *) indexPath
//{
//    // Return NO if you do not want the item to be re-orderable.
//    return YES;
//}

#pragma mark - UIPageViewControllerDataSource

- (UIViewController *)
pageViewController:                 (UIPageViewController *) pageViewController
viewControllerBeforeViewController: (UIViewController *)     viewController
{
    NSIndexPath *indexPath = self.currentCard;
    CardObject *card;
    BOOL isUserCard = NO;

    if (indexPath.section == 0) {
        return nil;

    } else if (indexPath.section == 1 && indexPath.row == 0) {
        indexPath = [NSIndexPath indexPathForRow: 0 inSection: 0];
        card = [APP_DELEGATE.blueModel activeUserCard];
        if (!card) return nil;
        isUserCard = YES;

    } else {
        indexPath = [NSIndexPath indexPathForRow: indexPath.row - 1 inSection: 1];
        NSNumber *cardId = self.activeDevice.visibleCards[indexPath.row];
        card = [CardObject objectForPrimaryKey: cardId];
    }

    UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
    BlueCardController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueCardController"];

    vc.card = card;
    vc.indexPath = indexPath;
    vc.isUserCard = isUserCard;

    return vc;
}

- (UIViewController *)
pageViewController:                (UIPageViewController *) pageViewController
viewControllerAfterViewController: (UIViewController *)     viewController
{
    NSIndexPath *indexPath = self.currentCard;
    CardObject *card;

    if (indexPath.section == 0) {
        if (self.activeDevice.visibleCards.count == 0) return nil;

        indexPath = [NSIndexPath indexPathForRow: 0 inSection: 1];
        NSNumber *cardId = self.activeDevice.visibleCards[indexPath.row];
        card = [CardObject objectForPrimaryKey: cardId];

    } else {
        indexPath = [NSIndexPath indexPathForRow: indexPath.row + 1 inSection: 1];
        if (indexPath.row >= self.activeDevice.visibleCards.count) return nil;

        NSNumber *cardId = self.activeDevice.visibleCards[indexPath.row];
        card = [CardObject objectForPrimaryKey: cardId];
    }

    UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
    BlueCardController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueCardController"];

    vc.card = card;
    vc.indexPath = indexPath;

    return vc;
}

// Erich: Removed these methods to disable the page controller entirely.
//- (NSInteger) presentationCountForPageViewController: (UIPageViewController *) pageViewController
//{
//    return 0;
//}
//
//- (NSInteger) presentationIndexForPageViewController: (UIPageViewController *) pageViewController
//{
//    return 0;
//}

#pragma mark - UIPageViewControllerDelegate

- (void)
pageViewController:              (UIPageViewController *)        pageViewController
willTransitionToViewControllers: (NSArray<UIViewController *> *) pendingViewControllers
{
    BlueCardController *vc = (BlueCardController *)pendingViewControllers[0];
    self.pendingCard = vc.indexPath;
}

- (void)
pageViewController:      (UIPageViewController *)        pageViewController
didFinishAnimating:      (BOOL)                          finished
previousViewControllers: (NSArray<UIViewController *> *) previousViewControllers
transitionCompleted:     (BOOL)                          completed
{
    NSNumber *cardId = self.activeDevice.visibleCards[self.pendingCard.row];

    [APP_DELEGATE.blueAnalytics viewCardBySwiping: [cardId integerValue]];

    self.currentCard = self.pendingCard;
}

// #pragma mark - Navigation
// 
// // In a storyboard-based application, you will often want to do a little preparation before navigation
// - (void)
//prepareForSegue: (UIStoryboardSegue *) segue
//sender:          (id)                  sender
//{
//     if ([segue isKindOfClass: [CCMPopupSegue class]]){
//         CCMPopupSegue *popupSegue = (CCMPopupSegue *)segue;
//         popupSegue.destinationBounds = CGRectMake(15, 72, 290, 435);
//     }
// }

@end
