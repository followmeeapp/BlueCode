//
//  BlueMainController.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueMainController.h"

#import <DigitsKit/DigitsKit.h>
#import <Realm/Realm.h>
#import <SDWebImage/UIImageView+WebCache.h>
#import <CCMPopup/CCMPopupTransitioning.h>
#import <CCMPopup/CCMPopupSegue.h>

#import "BlueApp.h"
#import "BlueModel.h"

#import "BlueRegisterController.h"
#import "BlueCardEditController.h"
#import "BlueCardController.h"

#import "EditCardController.h"

#import "UserObject.h"
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

@interface BlueMainController ()

@property (nonatomic, assign) MainStatus mainStatus;

@property (nonatomic, strong) RLMResults *array;
@property (nonatomic, strong) RLMNotificationToken *notification;

@property (nonatomic, strong) NSIndexPath *currentCard;

@property (nonatomic, strong) NSArray *networkKeys;
@property (nonatomic, strong) NSDictionary *networkInfo;

@end

@implementation BlueMainController

- (void) setupNetworkInfo
{
    self.networkKeys = @[
//        @(PokemonGoType), // No icon is shown for Pokémon Go.
        @(FacebookType),
        @(TwitterType),
        @(InstagramType),
        @(SnapchatType),
        @(GooglePlusType),
        @(YouTubeType),
        @(PinterestType),
        @(TumblrType),
        @(LinkedInType),
        @(PeriscopeType),
        @(VineType),
        @(SoundCloudType),
        @(SinaWeiboType),
        @(VKontakteType),
    ];

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

- (void) didCreateUser: (NSNotification *) note
{
    [self updateStatus];
}

- (void) didCreateCard: (NSNotification *) note
{
    [self updateStatus];
}

- (void) viewDidLoad
{
//    [[Digits sharedInstance] logOut];

    [super viewDidLoad];

    [self setupNetworkInfo];

    [self updateStatus];

    UserObject *user = [APP_DELEGATE.blueModel activeUser];
    if (user && user.cardId > 0) {
        [APP_DELEGATE startBluetoothAdvertising];
        [APP_DELEGATE startBluetoothDiscovery];
    }

    // Initialize Refresh Control
    UIRefreshControl *refreshControl = [[UIRefreshControl alloc] init];

    // Configure Refresh Control
    [refreshControl addTarget: self action: @selector(refresh:) forControlEvents: UIControlEventValueChanged];

    // Configure View Controller
    [self setRefreshControl: refreshControl];

    // Register for Refresh notification
    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(beginRefreshingTableView:) name: @"RefreshCards" object: nil];

    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(didCreateUser:) name: @"DidCreateUser" object: nil];
    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(didCreateCard:) name: @"DidCreateCard" object: nil];

    // Uncomment the following line to preserve selection between presentations.
    self.clearsSelectionOnViewWillAppear = YES;

    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    self.navigationItem.leftBarButtonItem = self.editButtonItem;

//    UIImage *image = [UIImage imageNamed: @"logo"];
//    UIImageView *imageview = [[UIImageView alloc] initWithImage: image];
//    self.navigationItem.titleView = imageview;

    UIImage *image = [UIImage imageNamed: @"logo"];
    UIButton *titleButton = [UIButton buttonWithType: UIButtonTypeCustom];
    [titleButton setImage: image forState: UIControlStateNormal];
    [titleButton setUserInteractionEnabled: YES];
    [titleButton addTarget: self action: @selector(viewUserCard:) forControlEvents: UIControlEventTouchUpInside];
    [titleButton setFrame: CGRectMake(0, 0, 167, 27)];
    self.navigationItem.titleView = titleButton;

    [self.navigationController.navigationBar setTintColor: [UIColor whiteColor]];
    [self.navigationController.navigationBar setTitleTextAttributes: @{ NSForegroundColorAttributeName: [UIColor whiteColor] }];

    self.array = [[CardObject allObjects] sortedResultsUsingProperty: @"timestamp" ascending: NO];

    // Set realm notification block
    __weak typeof(self) weakSelf = self;

    self.notification = [self.array addNotificationBlock: ^(RLMResults *data, RLMCollectionChange *changes, NSError *error) {
        if (error) {
            NSLog(@"Failed to open Realm on background worker: %@", error);
            return;
        }

        UITableView *tv = weakSelf.tableView;

        // Initial run of the query will pass nil for the change information
        if (!changes) {
            [tv reloadData];

        } else {
            // changes is non-nil, so we just need to update the tableview
            [tv beginUpdates];

            [tv reloadRowsAtIndexPaths: @[[NSIndexPath indexPathForRow: 0 inSection: 0]] withRowAnimation: UITableViewRowAnimationAutomatic];

            if (self.array.count > 1) {
                [tv deleteRowsAtIndexPaths: [changes deletionsInSection: 1]     withRowAnimation: UITableViewRowAnimationAutomatic];
                [tv insertRowsAtIndexPaths: [changes insertionsInSection: 1]    withRowAnimation: UITableViewRowAnimationAutomatic];
                [tv reloadRowsAtIndexPaths: [changes modificationsInSection: 1] withRowAnimation: UITableViewRowAnimationAutomatic];
            }
            
            [tv endUpdates];
        }
    }];
}

- (void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
}

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

- (void) refresh: (id) sender
{
    NSLog(@"Refreshing");

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1.5 * NSEC_PER_SEC), dispatch_get_main_queue(), ^(void) {
        NSLog(@"End Refreshing");
        [(UIRefreshControl *)sender endRefreshing];
    });
}

- (IBAction) viewUserCard: sender
{
    CardObject *userCard = [APP_DELEGATE.blueModel activeUserCard];
    if (!userCard) return;

    [self tableView: self.tableView didSelectRowAtIndexPath: [NSIndexPath indexPathForRow: 0 inSection: 0]];
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

            DGTAuthenticationConfiguration *configuration = [[DGTAuthenticationConfiguration alloc] initWithAccountFields: DGTAccountFieldsDefaultOptionMask];
            configuration.appearance = appearance;
            configuration.title = @"Join Blue";

            [[Digits sharedInstance] authenticateWithNavigationViewController: self.navigationController
                                     configuration:                            configuration
                                     completionViewController:                 completionViewController];

            return nil;

        } else if (_mainStatus == NeedToCreateCard) {
            UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
            BlueCardEditController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueCardEditController"];
            [self.navigationController pushViewController: vc animated: YES];
            return nil;

        } else {
            return indexPath;
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
    }

    return self.array.count == 0 ? 0 : self.array.count - 1;
}

- (void)
configureBlueCardCell: (BlueCardCell *) cell
withCardObject:        (CardObject *)   card
{
    cell.fullName.text = card.fullName;
    cell.location.text = card.location;

    cell.avatar.clipsToBounds = YES;
    cell.avatar.layer.cornerRadius = 32; // this value vary as per your desire
    cell.avatar.layer.borderColor = [UIColor whiteColor].CGColor;
    cell.avatar.layer.borderWidth = 1.0;

    if (card.backgroundURLString) {
        [cell.coverPhoto sd_setImageWithURL: [NSURL URLWithString: card.backgroundURLString]
                           placeholderImage: [UIImage imageNamed: @"default-cover-photo"]    ];

    } else {
        cell.coverPhoto.image = [UIImage imageNamed: @"default-cover-photo"];
    }

    if (card.avatarURLString) {
        [cell.avatar sd_setImageWithURL: [NSURL URLWithString: card.avatarURLString]
                       placeholderImage: [UIImage imageNamed: @"missing-avatar"]           ];

    } else {
        cell.avatar.image = [UIImage imageNamed: @"missing-avatar"];
    }

    // Set the social network icons correctly.
    __block NSInteger index = 0;
    [self.networkKeys enumerateObjectsUsingBlock: ^(NSNumber *networkType, NSUInteger idx, BOOL *stop) {
        NSDictionary *networkInfo = self.networkInfo[networkType];

        if ([[card valueForKey: networkInfo[@"property"]] boolValue]) {
            UIImageView *dropShadow = [cell viewWithTag: index + 100];
            dropShadow.hidden = NO;

            UIImageView *icon = [cell viewWithTag: index + 200];
            icon.hidden = NO;
            icon.image = [UIImage imageNamed: networkInfo[@"icon"]];

            index++;
        }

        if (index >= 5) *stop = YES;
    }];

    // Hide remaining icons if we have fewer than 5 social networks.
    while (index < 5) {
        UIImageView *dropShadow = [cell viewWithTag: index + 100];
        dropShadow.hidden = YES;

        UIImageView *icon = [cell viewWithTag: index + 200];
        icon.hidden = YES;

        index++;
    }
}

- (UITableViewCell *)
tableView:             (UITableView *) tableView
cellForRowAtIndexPath: (NSIndexPath *) indexPath
{
    if (indexPath.section == 0) {
        if (_mainStatus == NeedToJoin) {
            BlueJoinCell *cell = [tableView dequeueReusableCellWithIdentifier: JOIN_CELL forIndexPath: indexPath];

            // Configure the cell...
            cell.customLabel.text = @"Tap to Join Blue";
            return cell;

        } else if (_mainStatus == NeedToCreateCard) {
            BlueCreateCardCell *cell = [tableView dequeueReusableCellWithIdentifier: CREATE_CARD_CELL forIndexPath: indexPath];

            // Configure the cell...
//            cell.customLabel.text = @"Tap to Join Blue";
            return cell;

        } else if (_mainStatus == HaveCard) {
            CardObject *card = [APP_DELEGATE.blueModel activeUserCard];
            BlueCardCell *cell = [tableView dequeueReusableCellWithIdentifier: CARD_CELL forIndexPath: indexPath];

            [self configureBlueCardCell: cell withCardObject: card];

            return cell;
        }

    } else {
        CardObject *card = self.array[indexPath.row];
        BlueCardCell *cell = [tableView dequeueReusableCellWithIdentifier: CARD_CELL forIndexPath: indexPath];

        [self configureBlueCardCell: cell withCardObject: card];

        return cell;
    }
}

- (NSString *)
tableView:               (UITableView *) tableView
titleForHeaderInSection: (NSInteger)     section
{
    if (section == 0) return @"Your Card";
    else return @"Recent";
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
    // Set the background color of the View
    view.tintColor = APP_DELEGATE.black;

    // Text Color
    UITableViewHeaderFooterView *header = (UITableViewHeaderFooterView *)view;
    [header.textLabel setTextColor: [UIColor whiteColor]];
}

- (BOOL)
tableView:             (UITableView *) tableView
canEditRowAtIndexPath: (NSIndexPath *) indexPath
{
    if (indexPath.section == 0) return NO;
    else return YES;
}

- (void)
tableView:          (UITableView *)               tableView
commitEditingStyle: (UITableViewCellEditingStyle) editingStyle
forRowAtIndexPath:  (NSIndexPath *)               indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        CardObject *card = self.array[indexPath.row];

        RLMRealm *realm = [RLMRealm defaultRealm];

        [realm beginWriteTransaction];
        // TODO(Erich): Is there anything else to delete?
        [realm deleteObjects: card.networks];
        [realm deleteObject: card];
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
        card = self.array[indexPath.row];
    }

    if (card.version == 0) {
        // This is a Bluetooth-only card.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        // FIXME(Erich): Try and load the card again.

        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"No Internet?"
                                                      message:           @"We need to fetch the full card info from the Blue cloud. We'll do that once you have Internet access again."
                                                      delegate:          nil
                                                      cancelButtonTitle: @"OK"
                                                      otherButtonTitles: nil                                                            ];
        [alertView show];
        return;
#pragma clang diagnostic pop
    }

    self.currentCard = indexPath;

    UIPageViewController *vc = [[UIPageViewController alloc] initWithTransitionStyle: UIPageViewControllerTransitionStyleScroll
                                                             navigationOrientation:   UIPageViewControllerNavigationOrientationHorizontal
                                                             options:                 nil                                                ];

    vc.dataSource = self;
    vc.delegate = self;

    // TODO(Erich): Any other page view controller configuration?
    UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
    BlueCardController *cc = [sb instantiateViewControllerWithIdentifier: @"BlueCardController"];

    cc.card = card;

    [vc setViewControllers: @[cc] direction: UIPageViewControllerNavigationDirectionForward animated: NO completion: nil];

    CCMPopupTransitioning *popup = [CCMPopupTransitioning sharedInstance];
    popup.dismissableByTouchingBackground = YES;
    popup.destinationBounds = [[UIScreen mainScreen] bounds]; // CGRectMake(15, 72, 290, 435);
    popup.presentedController = vc;
    popup.presentingController = self;
    [self presentViewController: vc animated: YES completion: nil];
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

#pragma mark UIPageViewControllerDataSource

- (UIViewController *)
pageViewController:                 (UIPageViewController *) pageViewController
viewControllerBeforeViewController: (UIViewController *)     viewController
{
    NSIndexPath *indexPath = self.currentCard;
    CardObject *card;

    if (indexPath.section == 0) {
        return nil;

    } else if (indexPath.row == 0) {
        indexPath = [NSIndexPath indexPathForRow: 0 inSection: 0];
        card = [APP_DELEGATE.blueModel activeUserCard];
        self.currentCard = indexPath;

    } else {
        indexPath = [NSIndexPath indexPathForRow: indexPath.row - 1 inSection: 1];
        card = self.array[indexPath.row];
        self.currentCard = indexPath;
    }

    UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
    BlueCardController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueCardController"];

    vc.card = card;

    return vc;
}

- (UIViewController *)
pageViewController:                (UIPageViewController *) pageViewController
viewControllerAfterViewController: (UIViewController *)     viewController
{
    NSIndexPath *indexPath = self.currentCard;
    CardObject *card;

    if (indexPath.section == 0) {
        if (self.array.count <= 1) return nil;

        indexPath = [NSIndexPath indexPathForRow: 0 inSection: 1];
        card = self.array[indexPath.row];

    } else {
        if (indexPath.row == self.array.count - 1) return nil;

        indexPath = [NSIndexPath indexPathForRow: indexPath.row + 1 inSection: 1];
        card = self.array[indexPath.row];
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

#pragma mark UIPageViewControllerDelegate

- (void)
pageViewController:              (UIPageViewController *)        pageViewController
willTransitionToViewControllers: (NSArray<UIViewController *> *) pendingViewControllers
{
    BlueCardController *vc = (BlueCardController *)pendingViewControllers[0];
    self.currentCard = vc.indexPath;
}

//- (void)
//pageViewController:      (UIPageViewController *)        pageViewController
//didFinishAnimating:      (BOOL)                          finished
//previousViewControllers: (NSArray<UIViewController *> *) previousViewControllers
//transitionCompleted:     (BOOL)                          completed
//{
//}

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
