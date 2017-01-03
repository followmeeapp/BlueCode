//
//  BlueCardController.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueCardController.h"

#import <Crashlytics/Crashlytics.h>
#import <NSHash/NSData+NSHash.h>
#import <SDWebImage/UIImageView+WebCache.h>
#import <SVProgressHUD/SVProgressHUD.h>
#import <TOCropViewController/TOCropViewController.h>

#import "BlueApp.h"
#import "BlueModel.h"
#import "BlueClient.h"

#import "BlueIntroController.h"
#import "BlueNetworkEditController.h"

#import "CardObject.h"
#import "NetworkObject.h"

#import "ServerRequest.h"

#import "Blue-Swift.h"

#import "NNLHashids.h"

@import GoogleMobileAds;

@interface BlueCardController () <
    GADInterstitialDelegate,
    TOCropViewControllerDelegate,
    UITextFieldDelegate,
    UITextViewDelegate,
    UIImagePickerControllerDelegate,
    UINavigationControllerDelegate
>

@property (nonatomic, strong) NSDictionary *networkInfo;

@property (nonatomic, assign) NSInteger version;
@property (nonatomic, strong) RLMNotificationToken *token;

@property (nonatomic, strong) GADInterstitial *interstitial;
@property (nonatomic, strong) NSIndexPath *tappedIndexPath;

@property (nonatomic, assign) BOOL editingAvatar;
@property (nonatomic, assign) BOOL isEditingAgain;

@property (nonatomic, assign) BOOL isSavingCard;

@property (nonatomic, strong) NSArray *networkKeys;
@property (nonatomic, strong) NSArray *sortedNetworks;

@property (nonatomic, copy) NSString *uploadUrl;
@property (nonatomic, copy) NSString *authorizationToken;

@property (nonatomic, copy) NSString *avatarUrl;
@property (nonatomic, copy) NSString *coverPhotoUrl;

@property (nonatomic, assign) NSTimeInterval started;

@property (nonatomic, strong) NSTimer *timer;
@property (nonatomic, assign) BOOL didTimeout;

@property (nonatomic, assign) NSInteger requestId;

@property (nonatomic, strong) UIImagePickerController *picker;

@property (nonatomic, strong) NNLHashids *hashids;

@property (nonatomic, strong) NSData *savedAvatar;
@property (nonatomic, strong) NSData *savedCoverPhoto;

@end

//@interface SocialNetworkFlowLayout : UICollectionViewFlowLayout
//@end

@implementation BlueCardController

- (void) viewDidLoad
{
    [super viewDidLoad];

    self.hashids = [[NNLHashids alloc] initWithSalt: @"4320C0E8-0609-45C8-BCC7-D4027AA24445" minHashLength: 8];

    [self configureNetworkInfo];

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

    self.fullName.delegate = self;
    self.location.delegate = self;
    self.bio.delegate = self;

    self.shareButton.clipsToBounds = YES;
    self.shareButton.layer.cornerRadius = 5.0;
    self.shareButton.layer.borderColor = [UIColor whiteColor].CGColor;
    self.shareButton.layer.borderWidth = 1.0;

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

    if (_shouldEditUserCard) {
        [self disableRealmToken];
        [self configureBackButton];
        [self configureSaveButton];

        // From BlueCardEditController
        [self.bio setEditable: YES];

        // Need to set up the properties of our text fields.
        if (APP_DELEGATE.fullName) {
            self.fullName.text = APP_DELEGATE.fullName;
        }

        if (APP_DELEGATE.location) {
            self.location.text = APP_DELEGATE.location;
        }

        UIColor *placeholderColor = [UIColor colorWithWhite: 1.0 alpha: 1.0];
        self.fullName.attributedPlaceholder = [[NSAttributedString alloc] initWithString: @"Full Name" attributes: @{ NSForegroundColorAttributeName: placeholderColor }];
        self.location.attributedPlaceholder = [[NSAttributedString alloc] initWithString: @"Location"  attributes: @{ NSForegroundColorAttributeName: placeholderColor }];

        self.bio.text = APP_DELEGATE.bio;
        self.bio.textColor = [UIColor whiteColor]; // NOTE: Must come AFTER the text is set.

        self.avatar.clipsToBounds = YES;
        self.avatar.layer.cornerRadius = 151.0 / 2.0;
        self.avatar.layer.borderColor = [UIColor whiteColor].CGColor;
        self.avatar.layer.borderWidth = 1.0;

        if (APP_DELEGATE.croppedAvatar) {
            self.avatar.image = APP_DELEGATE.croppedAvatar;

        } else if (APP_DELEGATE.savedAvatar) {
            self.avatar.image = APP_DELEGATE.savedAvatar;

        } else {
            self.avatar.image = [UIImage imageNamed: @"missing-avatar"];
        }

        if (APP_DELEGATE.croppedCoverPhoto) {
            self.background.image = APP_DELEGATE.croppedCoverPhoto;

        } else if (APP_DELEGATE.savedCoverPhoto) {
            self.background.image = APP_DELEGATE.savedCoverPhoto;

        } else {
            self.background.image = [UIImage imageNamed: @"default-card-background"];
        }

//        self.opacityOverlay.alpha = APP_DELEGATE.backgroundOpacity;

        // From BlueSocialNetworksController
//        self.networks.collectionViewLayout = [[SocialNetworkFlowLayout alloc] init];

        [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(refreshNotificationIcons:) name: @"BlueRefreshNetworkIcons" object: nil];

        [self.networks reloadData];

    } else {
        [self createAndLoadInterstitial];
        [self displayCard];

        if (self.isEditableCard) {
            [self configureEditButton];

        } else if (self.isCardLink) {
            [self configureCloseButton];
        }

        // Don't try and refresh the welcome card.
        if (self.card.id == 0) return;

        RLMRealm *realm = [RLMRealm defaultRealm];
        self.token = [realm addNotificationBlock: ^(NSString *notification, RLMRealm *realm) {
            CardObject *card = self.card;
            if (!card) return;

            if (card.version > self.version) [self displayCard];
        }];

        NSData *cardRequestPacket = [ServerRequest newCardRequestWithId: [APP_DELEGATE.blueClient nextRequestId] cardId: self.card.id version: self.card.version];
        [APP_DELEGATE.blueClient sendRequest: cardRequestPacket];
    }
}

- (void) refreshNotificationIcons: (NSNotification *) note
{
    [self.networks reloadData];
}

- (void) disableRealmToken
{
    if (self.token) {
        [self.token stop];
        self.token = nil;
    }
}

- (void) dealloc
{
    [self disableRealmToken];
    [[NSNotificationCenter defaultCenter] removeObserver: self];
}

// from BlueCardController
- (void) displayCard
{
    [self.avatarButton setEnabled: NO];
    [self.avatarButton setHidden: YES];
    [self.backgroundButton setEnabled: NO];
    [self.backgroundButton setHidden: YES];
    [self.backgroundSettingsButton setEnabled: NO];
    [self.backgroundSettingsButton setHidden: YES];

    [self.bio setEditable: NO];

    CardObject *card = self.card;
    if (!card) return;

    self.avatar.clipsToBounds = YES;
    self.avatar.layer.cornerRadius = 156.0 / 2.0;
    self.avatar.layer.borderColor = [UIColor whiteColor].CGColor;
    self.avatar.layer.borderWidth = 1.0;

    if (self.isUserCard) {
        [self.avatar sd_setImageWithURL: APP_DELEGATE.localAvatarURL
                     placeholderImage:   [UIImage imageNamed: @"missing-avatar"]
                     options:            SDWebImageRefreshCached               ];

        [self.background sd_setImageWithURL: APP_DELEGATE.localCoverPhotoURL
                         placeholderImage:   [UIImage imageNamed: @"default-card-background"]
                         options:            SDWebImageRefreshCached                        ];

    } else {
        if (card.avatarURLString) {
            [self.avatar sd_setImageWithURL: [NSURL URLWithString: card.avatarURLString]
                         placeholderImage:   [UIImage imageNamed: @"missing-avatar"]   ];

        } else {
            self.avatar.image = [UIImage imageNamed: @"missing-avatar"];
        }

        if (card.backgroundURLString) {
            [self.background sd_setImageWithURL: [NSURL URLWithString: card.backgroundURLString]
                             placeholderImage:   [UIImage imageNamed: @"default-card-background"]];

        } else {
            self.background.image = [UIImage imageNamed: @"default-card-background"];
        }
    }

    self.fullName.text = card.fullName;
    self.location.text = card.location ? card.location : @"";
    self.location.placeholder = nil;
    self.bio.text = card.bio ? card.bio : @"";
    self.bio.textColor = [UIColor whiteColor]; // NOTE: Must come AFTER the text is set.

    self.opacityOverlay.alpha = card.backgroundOpacity;

    self.version = card.version;

    // TODO(Erich): Animate the network changes?
    // Set the social network icons correctly.
    NSMutableArray *networks = [NSMutableArray array];
    [self.networkKeys enumerateObjectsUsingBlock: ^(NSNumber *networkType, NSUInteger idx, BOOL *stop) {
        NSDictionary *networkInfo = self.networkInfo[networkType];

        if ([[card valueForKey: networkInfo[@"property"]] boolValue]) {
            [networks addObject: networkType];
        }
    }];

    self.sortedNetworks = networks;
    [self.networks reloadData];
}

- (void) configureBackButton
{
    [self.navigationItem.leftBarButtonItem setTarget: self];
    [self.navigationItem.leftBarButtonItem setAction: @selector(dismiss:)];
}

- (void) configureSaveButton
{
    UIBarButtonItem *menuButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem: UIBarButtonSystemItemSave
                                                           target:                      self
                                                           action:                      @selector(save:)         ];
    self.navigationItem.rightBarButtonItem = menuButton;
}

- (void) configureEditButton
{
    UIBarButtonItem *menuButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem: UIBarButtonSystemItemEdit
                                                           target:                      self
                                                           action:                      @selector(edit:)         ];
    self.navigationItem.rightBarButtonItem = menuButton;
}

- (void) configureCloseButton
{
    UIImage *image = [UIImage imageNamed: @"close-icon"];
    UIBarButtonItem *menuButton = [[UIBarButtonItem alloc] initWithImage: image
                                                           style:         UIBarButtonItemStylePlain
                                                           target:        self
                                                           action:        @selector(dismiss:)      ];
    self.navigationItem.rightBarButtonItem = menuButton;
}

#pragma mark Actions

- (IBAction) edit: sender
{
    if (self.shouldEditUserCard) return;

    [APP_DELEGATE.blueAnalytics editCard: self.card.id];

    UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
    BlueCardController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueCardController"];
    vc.isEditableCard = YES;
    vc.shouldEditUserCard = YES;

    [APP_DELEGATE prepareToEditUserCard];

    NSMutableArray *vcs = [[self.navigationController viewControllers] mutableCopy];
    NSUInteger lastVcIndex = [vcs count] - 1;
    if (lastVcIndex > 0) {
        [vcs replaceObjectAtIndex: lastVcIndex withObject: vc];
        [self.navigationController setViewControllers: vcs animated: NO];
    }
}

- (IBAction) share: sender
{
    if (self.shouldEditUserCard) {
        UIAlertController *alert = [UIAlertController alertControllerWithTitle: @"Sharing Unavailable"
                                                      message:                  @"You need to save your card first."
                                                      preferredStyle:           UIAlertControllerStyleActionSheet];

        UIAlertAction *okAction = [UIAlertAction actionWithTitle: @"OK" style: UIAlertActionStyleCancel handler: nil];

        [alert addAction: okAction];

        [self presentViewController: alert animated: YES completion: nil];

    } else {
        NSMutableArray *sharingItems = [NSMutableArray new];

        NSString *hash = [self.hashids encode: @(self.card.id)];
        NSString *URLString = [NSString stringWithFormat: @"https://blue.cards/%@", hash];

        // Handle welcome card specially.
        if (self.card.id == 0) {
            URLString = @"http://blue.social";
        }

        [sharingItems addObject: [NSURL URLWithString: URLString]];

        UIActivityViewController *activityController = [[UIActivityViewController alloc] initWithActivityItems: sharingItems applicationActivities: nil];

        activityController.completionWithItemsHandler = ^(NSString *activityType, BOOL completed, NSArray *returnedItems, NSError *activityError) {
            if (completed) {
                [APP_DELEGATE.blueAnalytics shareCard:  self.card.id
                                            withMethod: activityType
                                            URLString:  URLString
                                            isUserCard: self.isUserCard];
            }
        };

        [self presentViewController: activityController animated: YES completion: nil];
    }
}

- (IBAction) showBackgroundSettingsMenu: sender
{
    
}

// from BlueCardController
- (IBAction) dismiss: (id)sender
{
    if (_shouldEditUserCard) {
        UIAlertController *alert = [UIAlertController alertControllerWithTitle: @"You have unsaved changes."
                                                      message:                  @"If you don't save them, they'll be lost."
                                                      preferredStyle:           UIAlertControllerStyleActionSheet];

        UIAlertAction *saveAction = [UIAlertAction actionWithTitle: @"Save changes"
                                                   style:           UIAlertActionStyleDefault
                                                   handler:
        ^(UIAlertAction * _Nonnull action) {
            [self save: nil];
        }];

        UIAlertAction *discardAction = [UIAlertAction actionWithTitle: @"Discard changes"
                                                      style:           UIAlertActionStyleDefault
                                                      handler:
        ^(UIAlertAction * _Nonnull action) {
            [self dismissViewControllerAnimated: YES completion: nil];
        }];

        UIAlertAction *cancelAction = [UIAlertAction actionWithTitle: @"Cancel" style: UIAlertActionStyleCancel handler: nil];

        [alert addAction: saveAction];
        [alert addAction: discardAction];
        [alert addAction: cancelAction];

        [self presentViewController: alert animated: YES completion: nil];

    } else {
        [self dismissViewControllerAnimated: YES completion: ^{
            if (APP_DELEGATE.didShowIntro) {
                APP_DELEGATE.didShowIntro = NO; // reset
                [((BlueIntroController *)(APP_DELEGATE.window.rootViewController)) showIntro];
            }
        }];
    }
}

// from BlueCardEditController
- (IBAction) editAvatar: sender
{
    if (_shouldEditUserCard == NO) return;

    self.editingAvatar = YES;

    if (APP_DELEGATE.originalAvatar) {
        // Need to ask if we should get a new image, or edit the existing one.
        UIAlertController *alert = [UIAlertController alertControllerWithTitle: @"Avatar"
                                                      message:                  @"What would you like to do?"
                                                      preferredStyle:           UIAlertControllerStyleActionSheet];

        UIAlertAction *selectImageAction = [UIAlertAction actionWithTitle: @"Choose a new image"
                                                          style:           UIAlertActionStyleDefault
                                                          handler:
        ^(UIAlertAction *action) {
            [self selectImage];
        }];

        UIAlertAction *takePictureAction = [UIAlertAction actionWithTitle: @"Take a picture"
                                                          style:           UIAlertActionStyleDefault
                                                          handler:
        ^(UIAlertAction *action) {
            [self takePicture];
        }];

        UIAlertAction *editExistingAction = [UIAlertAction actionWithTitle: @"Edit the current image"
                                                           style:           UIAlertActionStyleDefault
                                                           handler:
        ^(UIAlertAction *action) {
            self.isEditingAgain = YES;
            [self editImage];
        }];

        UIAlertAction *removeExistingAction = [UIAlertAction actionWithTitle: @"Remove the current image"
                                                             style:           UIAlertActionStyleDefault
                                                             handler:
        ^(UIAlertAction *action) {
            APP_DELEGATE.originalAvatar = nil;
            APP_DELEGATE.croppedAvatar = nil;
            APP_DELEGATE.didUpdateAvatar = YES;
            self.avatar.image = [UIImage imageNamed: @"missing-avatar"];
        }];

        UIAlertAction *cancelAction = [UIAlertAction actionWithTitle: @"Cancel" style: UIAlertActionStyleCancel handler: nil];

        [alert addAction: selectImageAction];
        [alert addAction: takePictureAction];
        [alert addAction: editExistingAction];
        [alert addAction: removeExistingAction];
        [alert addAction: cancelAction];

        [self presentViewController: alert animated: YES completion: nil];

    } else {
        self.isEditingAgain = NO;

        // Need to ask if we should get a new image, or edit the existing one.
        UIAlertController *alert = [UIAlertController alertControllerWithTitle: @"Add Avatar"
                                                      message:                  @"What would you like to do?"
                                                      preferredStyle:           UIAlertControllerStyleActionSheet];

        UIAlertAction *selectImageAction = [UIAlertAction actionWithTitle: @"Choose an image from my library"
                                                          style:           UIAlertActionStyleDefault
                                                          handler:
        ^(UIAlertAction *action) {
            [self selectImage];
        }];

        UIAlertAction *takePictureAction = [UIAlertAction actionWithTitle: @"Take a picture"
                                                          style:           UIAlertActionStyleDefault
                                                          handler:
        ^(UIAlertAction *action) {
            [self takePicture];
        }];

        UIAlertAction *cancelAction = [UIAlertAction actionWithTitle: @"Cancel" style: UIAlertActionStyleCancel handler: nil];

        [alert addAction: selectImageAction];
        [alert addAction: takePictureAction];
        [alert addAction: cancelAction];

        [self presentViewController: alert animated: YES completion: nil];
    }
}

// from BlueCardEditController
- (IBAction) editCoverPhoto: sender
{
    if (_shouldEditUserCard == NO) return;

    self.editingAvatar = NO;

    if (APP_DELEGATE.originalCoverPhoto) {
        // Need to ask if we should get a new image, or edit the existing one.
        UIAlertController *alert = [UIAlertController alertControllerWithTitle: @"Cover Photo"
                                                      message:                  @"What would you like to do?"
                                                      preferredStyle:           UIAlertControllerStyleActionSheet];

        UIAlertAction *selectImageAction = [UIAlertAction actionWithTitle: @"Choose a new image"
                                                          style:           UIAlertActionStyleDefault
                                                          handler:
        ^(UIAlertAction *action) {
            [self selectImage];
        }];

        UIAlertAction *takePictureAction = [UIAlertAction actionWithTitle: @"Take a picture"
                                                          style:           UIAlertActionStyleDefault
                                                          handler:
        ^(UIAlertAction *action) {
            [self takePicture];
        }];

        UIAlertAction *editExistingAction = [UIAlertAction actionWithTitle: @"Edit the current image"
                                                           style:           UIAlertActionStyleDefault
                                                           handler:
        ^(UIAlertAction *action) {
            self.isEditingAgain = YES;
            [self editImage];
        }];

        UIAlertAction *removeExistingAction = [UIAlertAction actionWithTitle: @"Remove the current image"
                                                             style:           UIAlertActionStyleDefault
                                                             handler:
        ^(UIAlertAction *action) {
            APP_DELEGATE.originalCoverPhoto = nil;
            APP_DELEGATE.croppedCoverPhoto = nil;
            APP_DELEGATE.didUpdateCoverPhoto = YES;
            self.background.image = [UIImage imageNamed: @"default-card-background"];
        }];

        UIAlertAction *cancelAction = [UIAlertAction actionWithTitle: @"Cancel" style: UIAlertActionStyleCancel handler: nil];

        [alert addAction: selectImageAction];
        [alert addAction: takePictureAction];
        [alert addAction: editExistingAction];
        [alert addAction: removeExistingAction];
        [alert addAction: cancelAction];

        [self presentViewController: alert animated: YES completion: nil];

    } else {
        self.isEditingAgain = NO;

            // Need to ask if we should get a new image, or edit the existing one.
        UIAlertController *alert = [UIAlertController alertControllerWithTitle: @"Add Cover Photo"
                                                      message:                  @"What would you like to do?"
                                                      preferredStyle:           UIAlertControllerStyleActionSheet];

        UIAlertAction *selectImageAction = [UIAlertAction actionWithTitle: @"Choose an image from my library"
                                                          style:           UIAlertActionStyleDefault
                                                          handler:
        ^(UIAlertAction *action) {
            [self selectImage];
        }];

        UIAlertAction *takePictureAction = [UIAlertAction actionWithTitle: @"Take a picture"
                                                          style:           UIAlertActionStyleDefault
                                                          handler:
        ^(UIAlertAction *action) {
            [self takePicture];
        }];

        UIAlertAction *cancelAction = [UIAlertAction actionWithTitle: @"Cancel" style: UIAlertActionStyleCancel handler: nil];

        [alert addAction: selectImageAction];
        [alert addAction: takePictureAction];
        [alert addAction: cancelAction];

        [self presentViewController: alert animated: YES completion: nil];
    }
}

// from BlueSocialNetworksController
- (IBAction) save: sender
{
    if (_shouldEditUserCard == NO) return;

    if (self.isSavingCard) return;

    [self.fullName resignFirstResponder];
    [self.location resignFirstResponder];
    [self.bio resignFirstResponder];

    self.savedAvatar = nil;
    self.savedCoverPhoto = nil;

    NSString *fullName = APP_DELEGATE.fullName;
    if (!fullName || [fullName length] == 0) {
        ValidationAlertWithMessage(@"You need to at least provide your full name.");
        return;
    }

    if ([fullName length] > 127) {
        ValidationAlertWithMessage(@"Full name is too long.");
        return;
    }

    NSString *location = APP_DELEGATE.location;
    if (location && [location length] > 127) {
        ValidationAlertWithMessage(@"Location is too long.");
        return;
    }

    NSString *bio = APP_DELEGATE.bio;
    if (bio && [bio length] > 511) {
        ValidationAlertWithMessage(@"Bio is too big.");
        return;
    }

    self.isSavingCard = YES;

    // Make sure we have local copies of our images!
    [APP_DELEGATE saveTemporaryCardProperties];

    [SVProgressHUD setDefaultStyle: SVProgressHUDStyleCustom];
    [SVProgressHUD setDefaultMaskType: SVProgressHUDMaskTypeBlack];
    [SVProgressHUD setForegroundColor: [UIColor blackColor]];
    [SVProgressHUD setBackgroundColor: [UIColor whiteColor]];

    [SVProgressHUD show];

    self.didTimeout = NO; // Need to reset this each time.
    [self resetTimer: 7.0];

    NSURLSession *session = [NSURLSession sharedSession];
    NSURL *uploadInfoURL = [NSURL URLWithString: @"http://199.19.87.92:3004"];

    // If we haven't created a card yet, we need to force-save photos because
    // the user might have failed to save them the first time *sigh*.
    if (!APP_DELEGATE.isUpdatingCard) {
        APP_DELEGATE.didUpdateAvatar = YES;
        APP_DELEGATE.didUpdateCoverPhoto = YES;
    }

    if (APP_DELEGATE.didUpdateAvatar || APP_DELEGATE.didUpdateCoverPhoto) {
        // We need to get an upload URL.
        id task = [session downloadTaskWithURL: uploadInfoURL
                           completionHandler:
        ^(NSURL *location, NSURLResponse *response, NSError * error) {
            if (self.didTimeout) return;

            NSData *data = location ? [NSData dataWithContentsOfURL: location] : nil;

            if (data) {
                NSError *error = nil;
                NSDictionary *dict = [NSJSONSerialization JSONObjectWithData: data
                                                          options:            0
                                                          error:              &error];

                if (error) {
                    NSLog(@"JSON parse error: %@", error);
                    self.isSavingCard = NO;
                    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                        [self showError: @"Error uploading images!"];
                    });

                } else {
                    self.uploadUrl = dict[@"uploadUrl"];
                    self.authorizationToken = dict[@"authorizationToken"];

                    if (APP_DELEGATE.didUpdateAvatar) {
                        if (APP_DELEGATE.croppedAvatar) {
                            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                                [self uploadAvatar: UIImageJPEGRepresentation(APP_DELEGATE.croppedAvatar, 0.5)];
                            });

                        } else {
                            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                                [self didUploadAvatarToURL: nil];
                            });
                        }

                    } else if (APP_DELEGATE.isUpdatingCard) {
                        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                            [self didUploadAvatarToURL: APP_DELEGATE.card.avatarURLString];
                        });

                    } else {
                        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                            [self didUploadAvatarToURL: nil];
                        });
                    }
                }

            } else {
               dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                   [self showError: @"Cannot upload images!"];
               });
            }
        }];

        [task resume];

    } else {
        if (APP_DELEGATE.isUpdatingCard) {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                [self didUploadAvatarToURL: APP_DELEGATE.card.avatarURLString];
            });

        } else {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                [self didUploadAvatarToURL: nil];
            });
        }
    }
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
    if (_shouldEditUserCard) {
        return self.networkKeys.count;

    } else {
        return self.sortedNetworks.count;
    }
}

- (UICollectionViewCell *)
collectionView:         (UICollectionView *) collectionView
cellForItemAtIndexPath: (NSIndexPath *)      indexPath
{
    NSDictionary *props = nil;

    if (_shouldEditUserCard) {
        NSNumber *networkType = [self.networkKeys objectAtIndex: indexPath.row];
        props = self.networkInfo[networkType];


    } else {
        NSNumber *networkType = [self.sortedNetworks objectAtIndex: indexPath.row];
        props = self.networkInfo[networkType];
    }

    static NSString *cellIdentifier = @"CardNetworkCell";
    UICollectionViewCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier: cellIdentifier forIndexPath: indexPath];

    UIImageView *iconView = (UIImageView *)[cell viewWithTag: 1];
    iconView.image = [UIImage imageNamed: props[@"icon"]];

    if (_shouldEditUserCard) {
        UIImageView *dropShadowView = (UIImageView *)[cell viewWithTag: 2];

        NSNumber *networkType = [self.networkKeys objectAtIndex: indexPath.row];
        if (APP_DELEGATE.networks[networkType]) {
            iconView.alpha = 1.0;
            dropShadowView.alpha = 0.1;

        } else {
            iconView.alpha = 0.5;
            dropShadowView.alpha = 0.0;
        }
    }

    return cell;
}

- (void)
collectionView:           (UICollectionView *) collectionView
didSelectItemAtIndexPath: (NSIndexPath *)      indexPath
{
    if (_shouldEditUserCard) {
        NSNumber *networkType = [self.networkKeys objectAtIndex: indexPath.row];
        NSDictionary *props = self.networkInfo[networkType];

        UIStoryboard *sb = [UIStoryboard storyboardWithName: @"Main" bundle: nil];
        BlueNetworkEditController *vc = [sb instantiateViewControllerWithIdentifier: @"BlueNetworkEditController"];

        vc.cardId = self.card.id;
        vc.networkKey = props[@"key"];
        vc.networkName = props[@"name"];
        vc.networkSlug = props[@"slug"];
        if ([props[@"name"] isEqualToString: @"YouTube"]) {
            vc.promptText = @"YouTube Channel";

        } else {
            vc.promptText = [NSString stringWithFormat: @"%@ Username", props[@"name"]];
        }
        vc.backgroundColor = props[@"color"];
        vc.image = [UIImage imageNamed: props[@"largeIcon"]];

        [self.navigationController pushViewController: vc animated: YES];

    } else {
        CardObject *card = self.card;
        if (!card) return;

        NetworkObject *network = nil;
        NetworkType networkType = [[self.sortedNetworks objectAtIndex: indexPath.row] integerValue];

        for (NSInteger idx=0, len=card.networks.count; idx<len; ++idx) {
            NetworkObject *tmpNetwork = card.networks[idx];
            if (tmpNetwork.type == networkType) {
                network = tmpNetwork;
                break;
            }
        }

        if (!network) return;

        NSInteger randomNumber = arc4random() % 100;

        BOOL wouldLaunch = [self wouldLaunchAppForCard: card network: network onlyIfUserHasNetwork: NO];
        if (wouldLaunch && randomNumber < 17) {
            if (self.interstitial.isReady) {
                self.tappedIndexPath = indexPath;
                [self.interstitial presentFromRootViewController: self];
                return;

            } else {
                NSLog(@"Wanted to display an ad, but ad wasn't ready.");
                [APP_DELEGATE.blueAnalytics adUnavailable];
            }
        }

        [self launchAppForCard: card network: network onlyIfUserHasNetwork: NO];
    }
}

- (BOOL)
wouldLaunchAppForCard: (CardObject *)    card
network:               (NetworkObject *) network
onlyIfUserHasNetwork:  (BOOL)            onlyIfUserHasNetwork
{
    return YES;
//    NSDictionary *networkInfo = self.networkInfo[@(network.type)];
//
//    if (![card[networkInfo[@"property"]] boolValue]) {
//        return NO; // Card does not have this network set up.
//    }
//
//    UserObject *user = [APP_DELEGATE.blueModel activeUser];
//    CardObject *userCard = [APP_DELEGATE.blueModel activeUserCard];
//    if (!user) {
//        return onlyIfUserHasNetwork ? YES : NO;
//
//    } else {
//        if (![userCard[networkInfo[@"property"]] boolValue]) {
//            if (onlyIfUserHasNetwork) {
//                return NO; // User does not have this social network configured.
//
//            } else {
//                return NO;
//            }
//        }
//    }
//
//    switch (network.type) {
//        case FacebookType: {
//            if (!card.hasFacebook) return NO;
//        } break;
//
//        case TwitterType: {
//            if (!card.hasTwitter) return NO;
//        } break;
//
//        case InstagramType: {
//            if (!card.hasInstagram) return NO;
//        }
//        case SnapchatType: {
//            if (!card.hasSnapchat) return NO;
//        } break;
//
//        case GooglePlusType: {
//            if (!card.hasGooglePlus) return NO;
//        } break;
//
//        case YouTubeType: {
//            if (!card.hasYouTube) return NO;
//        } break;
//
//        case PinterestType: {
//            if (!card.hasPinterest) return NO;
//        } break;
//
//        case TumblrType: {
//            if (!card.hasTumblr) return NO;
//        } break;
//
//        case LinkedInType: {
//            if (!card.hasLinkedIn) return NO;
//        } break;
//
//        case PeriscopeType: {
//            if (!card.hasPeriscope) return NO;
//        } break;
//
//        case VineType: {
//            if (!card.hasVine) return NO;
//        } break;
//
//        case SoundCloudType: {
//            if (!card.hasSoundCloud) return NO;
//        } break;
//
//        case SinaWeiboType: {
//            if (!card.hasSinaWeibo) return NO;
//        } break;
//
//        case VKontakteType: {
//            if (!card.hasVKontakte) return NO;
//        } break;
//    }
//
//    return YES; // We would do a deep link.
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

//    CardObject *userCard = [APP_DELEGATE.blueModel activeUserCard];
//    if (!userCard) {
//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wdeprecated-declarations"
//        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Members only."
//                                                      message:           @"You need to Join Blue before you can view other user's cards."
//                                                      delegate:          nil
//                                                      cancelButtonTitle: @"OK"
//                                                      otherButtonTitles: nil                                                            ];
//        [alertView show];
//        return onlyIfUserHasNetwork ? YES : NO;
//#pragma clang diagnostic pop
//
//    } else {
//        if (![userCard[networkInfo[@"property"]] boolValue]) {
//            if (onlyIfUserHasNetwork) {
//                return NO; // User does not have this social network configured.
//
//            } else {
//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wdeprecated-declarations"
//                // TODO(Erich): Allow the user to go to the App Store to install the app, or edit their own card.
//                UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Whoops!"
//                                                              message:           @"You don't have this social network enabled in your own card."
//                                                              delegate:          nil
//                                                              cancelButtonTitle: @"OK"
//                                                              otherButtonTitles: nil                                                           ];
//                [alertView show];
//                return NO;
//#pragma clang diagnostic pop
//            }
//        }
//    }

    [APP_DELEGATE.blueAnalytics openSocialNetwork: networkInfo[@"name"]
                                withUsername:      network.username
                                fromCard:          card.id            ];

    switch (network.type) {
        case FacebookType: {
            if (!card.hasFacebook) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == FacebookType) {
                    NSString *urlString = [NSString stringWithFormat: @"fb://profile?id=%@", network.username];
                    NSURL *appURL = [NSURL URLWithString: urlString];
                    if (![[UIApplication sharedApplication] openURL: appURL]) {
                        urlString = [NSString stringWithFormat: @"https://www.facebook.com/%@", network.username];
                        NSURL *webURL = [NSURL URLWithString: urlString];
                        [[UIApplication sharedApplication] openURL: webURL];
                    }
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
                    NSCharacterSet *notDigits = [[NSCharacterSet decimalDigitCharacterSet] invertedSet];
                    if ([network.username rangeOfCharacterFromSet: notDigits].location == NSNotFound) {
                        // TODO(Erich): we can deep link with an actual numeric id on iOS
                        NSString *urlString = [NSString stringWithFormat: @"https://plus.google.com/%@", network.username];
                        [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];

                    } else {
                        NSString *urlString = [NSString stringWithFormat: @"https://plus.google.com/u/0/+%@", network.username];
                        [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
                    }
                }
            }
        } break;

        case YouTubeType: {
            if (!card.hasYouTube) return NO;

            for (NetworkObject *network in card.networks) {
                if (network.type == YouTubeType) {
                    NSString *urlString = [NSString stringWithFormat: @"https://www.youtube.com/channel/%@", network.username];
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

//        case VineType: {
//            if (!card.hasVine) return NO;
//
//            for (NetworkObject *network in card.networks) {
//                if (network.type == VineType) {
//                    NSString *urlString = [NSString stringWithFormat: @"https://vine.co/%@", network.username];
//                    [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
//                }
//            }
//        } break;

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

        default:
            ; // Do nothing
    }

    return YES; // We did a deep link.
}

#pragma mark BlueCardEditController

- (void) takePicture
{
    self.picker = [[UIImagePickerController alloc] init];

    self.picker.delegate = self;
    self.picker.allowsEditing = NO;
    self.picker.sourceType = UIImagePickerControllerSourceTypeCamera;

    [self presentViewController: self.picker animated: YES completion: NULL];
}

- (void) selectImage
{
    self.picker = [[UIImagePickerController alloc] init];

    self.picker.delegate = self;
    self.picker.allowsEditing = NO;
    self.picker.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;

    [self presentViewController: self.picker animated: YES completion: NULL];
}

- (void) editImage
{
    TOCropViewController *cropViewController = nil;

    if (self.editingAvatar) {
        cropViewController = [[TOCropViewController alloc] initWithCroppingStyle: TOCropViewCroppingStyleCircular image: APP_DELEGATE.originalAvatar];

        if (self.isEditingAgain) {
            cropViewController.imageCropFrame = APP_DELEGATE.croppedAvatarRect;
            cropViewController.angle = APP_DELEGATE.croppedAvatarAngle;
        }

    } else {
        cropViewController = [[TOCropViewController alloc] initWithImage: APP_DELEGATE.originalCoverPhoto];
        cropViewController.aspectRatioPreset = TOCropViewControllerAspectRatioPresetCustom;
        cropViewController.customAspectRatio = CGSizeMake(9.0, 16.0);
        cropViewController.aspectRatioLockEnabled = YES;

        if (self.isEditingAgain) {
            cropViewController.imageCropFrame = APP_DELEGATE.croppedCoverPhotoRect;
            cropViewController.angle = APP_DELEGATE.croppedCoverPhotoAngle;
        }
    }

    cropViewController.delegate = self;

    [self presentViewController: cropViewController animated: YES completion: nil];
}

#pragma mark TOCropViewControllerDelegate

- (void)
cropViewController:     (TOCropViewController *) cropViewController
didCropToCircularImage: (UIImage *)              image
withRect:               (CGRect)                 cropRect
angle:                  (NSInteger)              angle
{
    // 'image' is the newly cropped, circular version of the original image
    if (self.editingAvatar) {
        APP_DELEGATE.croppedAvatar = [self imageWithImage: image scaledToSize: CGSizeMake(302, 302)];
        self.avatar.image = APP_DELEGATE.croppedAvatar;
        APP_DELEGATE.croppedAvatarRect = cropRect;
        APP_DELEGATE.croppedAvatarAngle = angle;
        APP_DELEGATE.didUpdateAvatar = YES;
    }

    [cropViewController dismissViewControllerAnimated: YES completion: NULL];
}

- (void)
cropViewController: (TOCropViewController *) cropViewController
didCropToImage:     (UIImage *)              image
withRect:           (CGRect)                 cropRect
angle:              (NSInteger)              angle
{
    // 'image' is the newly cropped, aspect-ratio version of the original image
    if (!self.editingAvatar) {
        APP_DELEGATE.croppedCoverPhoto = [self imageWithImage: image scaledToSize: CGSizeMake(540, 960)];;
        self.background.image = APP_DELEGATE.croppedCoverPhoto;
        APP_DELEGATE.croppedCoverPhotoRect = cropRect;
        APP_DELEGATE.croppedCoverPhotoAngle = angle;
        APP_DELEGATE.didUpdateCoverPhoto = YES;
    }

    [cropViewController dismissViewControllerAnimated: YES completion: NULL];
}

- (void)
cropViewController: (TOCropViewController *) cropViewController
didFinishCancelled: (BOOL)                   cancelled
{
    // We need to remove the image if we didn't end up using it.
    if (self.editingAvatar) {
        if (self.avatar.image != APP_DELEGATE.croppedAvatar) {
            APP_DELEGATE.originalAvatar = nil;
            APP_DELEGATE.croppedAvatar = nil;
            APP_DELEGATE.didUpdateAvatar = YES;
            self.avatar.image = [UIImage imageNamed: @"missing-avatar"];
        }

    } else {
        if (self.background.image != APP_DELEGATE.croppedCoverPhoto) {
            APP_DELEGATE.originalCoverPhoto = nil;
            APP_DELEGATE.croppedCoverPhoto = nil;
            APP_DELEGATE.didUpdateCoverPhoto = YES;
            self.background.image = [UIImage imageNamed: @"default-card-background"];
        }
    }

    [cropViewController dismissViewControllerAnimated: YES completion: NULL];
}

#pragma mark UIImagePickerControllerDelegate

- (void)
imagePickerController:         (UIImagePickerController *) picker
didFinishPickingMediaWithInfo: (NSDictionary *)            info
{
    [picker dismissViewControllerAnimated: YES completion: ^{
        UIImage *image  = [info objectForKey: UIImagePickerControllerOriginalImage];

        if (self.editingAvatar) {
            APP_DELEGATE.originalAvatar = image;

        } else {
            APP_DELEGATE.originalCoverPhoto = image;
        }

        [self editImage];
    }];
}

- (void)
imagePickerControllerDidCancel: (UIImagePickerController *) picker
{
    // We need to remove the image if we didn't end up using it.
    if (self.editingAvatar) {
        if (self.avatar.image != APP_DELEGATE.croppedAvatar) {
            APP_DELEGATE.originalAvatar = nil;
            APP_DELEGATE.croppedAvatar = nil;
            self.avatar.image = [UIImage imageNamed: @"missing-avatar"];
        }

    } else {
        if (self.background.image != APP_DELEGATE.croppedCoverPhoto) {
            APP_DELEGATE.originalCoverPhoto = nil;
            APP_DELEGATE.croppedCoverPhoto = nil;
            self.background.image = [UIImage imageNamed: @"default-card-background"];
        }
    }

    [picker dismissViewControllerAnimated: YES completion: NULL];
}

#pragma mark UITextFieldDelegate

- (BOOL) textFieldShouldBeginEditing: (UITextField *) textField
{
    return _shouldEditUserCard;
}

- (BOOL) textFieldShouldReturn: (UITextField *) textField
{
    if (textField == self.fullName) {
        [textField resignFirstResponder];
        [self.location becomeFirstResponder];

    } else {
        [textField resignFirstResponder];
        [self.bio becomeFirstResponder];
    }

    return NO;
}

- (void) textFieldDidEndEditing: (UITextField *) textField
{
    if (textField == self.fullName) {
        APP_DELEGATE.fullName = textField.text;

    } else if (textField == self.location) {
        APP_DELEGATE.location = textField.text;
    }
}

#pragma mark BlueSocialNetworksController

- (void) didTimeout: (NSTimer *) timer
{
    self.didTimeout = YES;

    [self showError: APP_DELEGATE.isUpdatingCard ? @"Save timed out!" : @"Create timed out!"];
}

- (void) invalidateTimer
{
    if (!self.timer) return;

    [self.timer invalidate];
    self.timer = nil;
}

- (void) resetTimer: (NSTimeInterval) duration
{
    [self invalidateTimer];

    self.timer = [NSTimer timerWithTimeInterval: duration
                          target:                self
                          selector:              @selector(didTimeout:)
                          userInfo:              nil
                          repeats:               NO                   ];

    [[NSRunLoop currentRunLoop] addTimer: self.timer forMode: NSRunLoopCommonModes];
}

- (void) showError: (NSString *) message
{
    [self invalidateTimer];
    self.isSavingCard = NO;

    [SVProgressHUD showErrorWithStatus: message];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [SVProgressHUD dismiss];
    });
}

- (void) uploadAvatar: (NSData *) uploadData
{
    [self resetTimer: 30.0];

    self.savedAvatar = uploadData;

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
        if (self.didTimeout) return;

        if (data) {
            NSError *error = nil;
            NSDictionary *dict = [NSJSONSerialization JSONObjectWithData: data
                                                      options:            0
                                                      error:              &error];

            if (error) {
                NSLog(@"JSON parse error: %@", error);
                self.isSavingCard = NO;
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                    [self showError: @"Failed to upload avatar!"];
                });

            } else {
                NSNumber *status = dict[@"status"];
                if (status && ![status isEqualToNumber: @(200)]) {
                    NSLog(@"Avatar upload error: %@", dict);
                    self.isSavingCard = NO;
                    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                        [self showError: @"Failed to upload avatar!"];
                    });

                } else {
                    NSString *urlPrefix = @"https://f001.backblaze.com/b2api/v1/b2_download_file_by_id?fileId=";
                    NSString *fileId = dict[@"fileId"];
                    NSString *downloadUrl = [NSString stringWithFormat: @"%@%@", urlPrefix, fileId];

                    NSLog(@"downloadUrl = %@", downloadUrl);

                    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                        [self didUploadAvatarToURL: downloadUrl];
                    });
                }
            }

       } else {
           dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
               [self showError: @"Uploading avatar failed!"];
           });
       }
    }];

    [task resume];

    // Save locally, too.
    [uploadData writeToURL: APP_DELEGATE.localAvatarURL atomically: YES];
}

- (void) didUploadAvatarToURL: (NSString *) url
{
    self.avatarUrl = url;

    if (APP_DELEGATE.didUpdateCoverPhoto) {
        if (APP_DELEGATE.croppedCoverPhoto) {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                [self uploadCoverPhoto: UIImageJPEGRepresentation(APP_DELEGATE.croppedCoverPhoto, 0.5)];
            });

        } else {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                [self didUploadCoverPhotoToURL: nil];
            });
        }

    } else if (APP_DELEGATE.isUpdatingCard) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
            [self didUploadCoverPhotoToURL: APP_DELEGATE.card.backgroundURLString];
        });

    } else {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
            [self didUploadCoverPhotoToURL: nil];
        });
    }
}

- (void) uploadCoverPhoto: (NSData *) uploadData
{
    [self resetTimer: 30.0];

    self.savedCoverPhoto = uploadData;

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
        if (self.didTimeout) return;

        if (data) {
            NSError *error = nil;
            NSDictionary *dict = [NSJSONSerialization JSONObjectWithData: data
                                                      options:            0
                                                      error:              &error];

            if (error) {
                NSLog(@"JSON parse error: %@", error);
                self.isSavingCard = NO;
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                    [self showError: @"Failed to upload cover photo!"];
                });

            } else {
                NSNumber *status = dict[@"status"];
                if (status && ![status isEqualToNumber: @(200)]) {
                    NSLog(@"Cover photo upload error: %@", dict);
                    self.isSavingCard = NO;
                    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                        [self showError: @"Failed to upload cover photo!"];
                    });

                } else {
                    NSString *urlPrefix = @"https://f001.backblaze.com/b2api/v1/b2_download_file_by_id?fileId=";
                    NSString *fileId = dict[@"fileId"];
                    NSString *downloadUrl = [NSString stringWithFormat: @"%@%@", urlPrefix, fileId];

                    NSLog(@"downloadUrl = %@", downloadUrl);

                    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                        [self didUploadCoverPhotoToURL: downloadUrl];
                    });
                }
            }

        } else {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^{
                [self showError: @"Uploading cover photo failed!"];
            });
        }
    }];

    [task resume];

    // Save locally, too.
    [uploadData writeToURL: APP_DELEGATE.localCoverPhotoURL atomically: YES];
}

- (void) didUploadCoverPhotoToURL: (NSString *) url
{
    self.coverPhotoUrl = url;

    [self resetTimer: 7.0];

    NSMutableDictionary *properties = [NSMutableDictionary dictionary];

    properties[@"fullName"] = APP_DELEGATE.fullName;
    properties[@"networks"] = APP_DELEGATE.networks;

    if (APP_DELEGATE.location) properties[@"location"] = APP_DELEGATE.location;
    if (APP_DELEGATE.bio) properties[@"bio"] = APP_DELEGATE.bio;

    if (self.avatarUrl) properties[@"avatarURLString"] = self.avatarUrl;
    if (self.coverPhotoUrl) properties[@"backgroundURLString"] = self.coverPhotoUrl;

    self.requestId = [APP_DELEGATE.blueClient nextRequestId];

    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(responseDidError:) name: @"ResponseDidError" object: nil];

    if (APP_DELEGATE.isUpdatingCard) {
        // Now update the actual card.
        CardObject *card = [APP_DELEGATE.blueModel activeUserCard];

        properties[@"id"] = @(card.id);
        properties[@"version"] = @(card.version);

        [APP_DELEGATE.blueClient sendRequest: [ServerRequest newUpdateCardRequestWithId: self.requestId properties: properties]];

        [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(didCreateCard:) name: @"DidUpdateCard" object: @(card.id)];

    } else {
        // Now create the actual card and send it.
        self.started = [NSDate timeIntervalSinceReferenceDate];
        [APP_DELEGATE.blueClient sendRequest: [ServerRequest newCreateCardRequestWithId: self.requestId properties: properties]];

        [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(didCreateCard:) name: @"DidCreateCard" object: nil];
    }
}

- (void) didCreateCard: (NSNotification *) note
{
    // FIXME(Erich): Should we also cancel the request somehow?

    if (self.didTimeout) return;

    [self invalidateTimer];

    if (self.savedAvatar) {
        [self.savedAvatar writeToURL: APP_DELEGATE.localAvatarURL atomically: YES];
    }

    if (self.savedCoverPhoto) {
        [self.savedCoverPhoto writeToURL: APP_DELEGATE.localCoverPhotoURL atomically: YES];
    }

    if (APP_DELEGATE.isUpdatingCard) {
        [SVProgressHUD showSuccessWithStatus: @"Saved!"];

        [APP_DELEGATE stopBluetoothAdvertising];

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [SVProgressHUD dismiss];

            [APP_DELEGATE startBluetoothAdvertising];

            [self.navigationController popToRootViewControllerAnimated: YES];
        });

    } else {
        [SVProgressHUD showSuccessWithStatus: @"Created!"];

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [SVProgressHUD dismiss];

            [APP_DELEGATE startBluetoothAdvertising];

            [self.navigationController popToRootViewControllerAnimated: YES];
        });
    }
}

- (void) responseDidError: (NSNotification *) note
{
    NSInteger requestId = [note.userInfo[@"requestId"] integerValue];
    if (requestId != self.requestId) return;

    [self invalidateTimer];

    [self showError: APP_DELEGATE.isUpdatingCard ? @"Error creating card!" : @"Error updating card!"];
}

#pragma mark BlueBioController
#pragma mark UITextViewDelegate

- (void) textViewDidEndEditing: (UITextView *) textView
{
    APP_DELEGATE.bio = textView.text;
}

#pragma mark Misc

- (void) configureNetworkInfo
{
    self.networkInfo = @{
        @(FacebookType): @{
            @"property": @"hasFacebook",
            @"name": @"Facebook",
            @"slug": @"facebook",
            @"icon": @"facebook-icon",
            @"largeIcon": @"facebook-large",
            @"color": [UIColor colorWithRed: (58.0/255.0) green: (87.0/255.0) blue: (154.0/255.0) alpha: 1.0],
            @"key": @(FacebookType),
        },
        @(TwitterType): @{
            @"property": @"hasTwitter",
            @"name": @"Twitter",
            @"slug": @"twitter",
            @"icon": @"twitter-icon",
            @"largeIcon": @"twitter-large",
            @"color": [UIColor colorWithRed: (80.0/255.0) green: (170.0/255.0) blue: (241.0/255.0) alpha: 1.0],
            @"key": @(TwitterType),
        },
        @(InstagramType): @{
            @"property": @"hasInstagram",
            @"name": @"Instagram",
            @"slug": @"instagram",
            @"icon": @"instagram-icon",
            @"largeIcon": @"instagram-large",
            @"color": [UIColor colorWithRed: (60.0/255.0) green: (113.0/255.0) blue: (157.0/255.0) alpha: 1.0],
            @"key": @(InstagramType),
        },
        @(SnapchatType): @{
            @"property": @"hasSnapchat",
            @"name": @"Snapchat",
            @"slug": @"snapchat",
            @"icon": @"snapchat-icon",
            @"largeIcon": @"snapchat-large",
            @"color": [UIColor colorWithRed: (254.0/255.0) green: (255.0/255.0) blue: (0.0/255.0) alpha: 1.0],
            @"key": @(SnapchatType),
        },
        @(GooglePlusType): @{
            @"property": @"hasGooglePlus",
            @"name": @"Google+",
            @"slug": @"googleplus",
            @"icon": @"googleplus-icon",
            @"largeIcon": @"googleplus-large",
            @"color": [UIColor colorWithRed: (225.0/255.0) green: (73.0/255.0) blue: (50.0/255.0) alpha: 1.0],
            @"key": @(GooglePlusType),
        },
        @(YouTubeType): @{
            @"property": @"hasYouTube",
            @"name": @"YouTube",
            @"slug": @"youtube",
            @"icon": @"youtube-icon",
            @"largeIcon": @"youtube-large",
            @"color": [UIColor colorWithRed: (234.0/255.0) green: (39.0/255.0) blue: (27.0/255.0) alpha: 1.0],
            @"key": @(YouTubeType),
        },
        @(PinterestType): @{
            @"property": @"hasPinterest",
            @"name": @"Pinterest",
            @"slug": @"pinterest",
            @"icon": @"pinterest-icon",
            @"largeIcon": @"pinterest-large",
            @"color": [UIColor colorWithRed: (208.0/255.0) green: (26.0/255.0) blue: (31.0/255.0) alpha: 1.0],
            @"key": @(PinterestType),
        },
        @(TumblrType): @{
            @"property": @"hasTumblr",
            @"name": @"Tumblr",
            @"slug": @"tumblr",
            @"icon": @"tumblr-icon",
            @"largeIcon": @"tumblr-large",
            @"color": [UIColor colorWithRed: (52.0/255.0) green: (69.0/255.0) blue: (93.0/255.0) alpha: 1.0],
            @"key": @(TumblrType),
        },
        @(LinkedInType): @{
            @"property": @"hasLinkedIn",
            @"name": @"LinkedIn",
            @"slug": @"linkedin",
            @"icon": @"linkedin-icon",
            @"largeIcon": @"linkedin-large",
            @"color": [UIColor colorWithRed: (0.0/255.0) green: (116.0/255.0) blue: (182.0/255.0) alpha: 1.0],
            @"key": @(LinkedInType),
        },
        @(PeriscopeType): @{
            @"property": @"hasPeriscope",
            @"name": @"Periscope",
            @"slug": @"periscope",
            @"icon": @"periscope-icon",
            @"largeIcon": @"periscope-large",
            @"color": [UIColor colorWithRed: (53.0/255.0) green: (163.0/255.0) blue: (198.0/255.0) alpha: 1.0],
            @"key": @(PeriscopeType),
        },
//        @(VineType): @{
//            @"property": @"hasVine",
//            @"name": @"Vine",
//            @"slug": @"vine",
//            @"icon": @"vine-icon",
//            @"largeIcon": @"vine-large",
//            @"color": [UIColor colorWithRed: (0.0/255.0) green: (182.0/255.0) blue: (135.0/255.0) alpha: 1.0],
//            @"key": @(VineType),
//        },
        @(SoundCloudType): @{
            @"property": @"hasSoundCloud",
            @"name": @"SoundCloud",
            @"slug": @"soundcloud",
            @"icon": @"soundcloud-icon",
            @"largeIcon": @"soundcloud-large",
            @"color": [UIColor colorWithRed: (255.0/255.0) green: (136.0/255.0) blue: (0.0/255.0) alpha: 1.0],
            @"key": @(SoundCloudType),
        },
        @(SinaWeiboType): @{
            @"property": @"hasSinaWeibo",
            @"name": @"Sina Weibo",
            @"slug": @"sina-weibo",
            @"icon": @"weibo-icon",
            @"largeIcon": @"weibo-large",
            @"color": [UIColor colorWithRed: (182.0/255.0) green: (48.0/255.0) blue: (47.0/255.0) alpha: 1.0],
            @"key": @(SinaWeiboType),
        },
        @(VKontakteType): @{
            @"property": @"hasVKontakte",
            @"name": @"VKontakte",
            @"slug": @"vkontakte",
            @"icon": @"vk-icon",
            @"largeIcon": @"vk-large",
            @"color": [UIColor colorWithRed: (75.0/255.0) green: (117.0/255.0) blue: (163.0/255.0) alpha: 1.0],
            @"key": @(VKontakteType),
        },
    };
}

- (void) createAndLoadInterstitial
{
    if (self.interstitial) {
        self.interstitial.delegate = nil;
        self.interstitial = nil;
    }

    self.interstitial = [[GADInterstitial alloc] initWithAdUnitID: @"ca-app-pub-9762460744660147/8651547314"];
    self.interstitial.delegate = self;

    GADRequest *request = [GADRequest request];
    request.testDevices = @[kGADSimulatorID]; //, @"7b369ae0c889849db673219d3c4b3b44"];

    [self.interstitial loadRequest: request];
}

#pragma mark GADInterstitialDelegate

/// Called before the interstitial is to be animated off the screen.
- (void) interstitialWillDismissScreen: (GADInterstitial *) ad
{
    CardObject *card = self.card;
    if (!card) return;

    NetworkObject *network = nil;
    NetworkType networkType = [[self.sortedNetworks objectAtIndex: self.tappedIndexPath.row] integerValue];

    for (NSInteger idx=0, len=card.networks.count; idx<len; ++idx) {
        NetworkObject *tmpNetwork = card.networks[idx];
        if (tmpNetwork.type == networkType) {
            network = tmpNetwork;
            break;
        }
    }

    if (!network) return;

    [self createAndLoadInterstitial];

    [self launchAppForCard: self.card network: network onlyIfUserHasNetwork: NO];
}

/// Called just before the application will background or terminate because the user clicked on an
/// ad that will launch another application (such as the App Store). The normal
/// UIApplicationDelegate methods, like applicationDidEnterBackground:, will be called immediately
/// before this.
- (void) interstitialWillLeaveApplication: (GADInterstitial *) ad
{
    NSLog(@"interstitialWillLeaveApplication:");
}

#pragma mark Image Resize Utility

// See http://stackoverflow.com/a/2658801
- (UIImage *)
imageWithImage: (UIImage *) image
scaledToSize:   (CGSize)    newSize
{
    //UIGraphicsBeginImageContext(newSize);
    // In next line, pass 0.0 to use the current device's pixel scaling factor (and thus account for Retina resolution).
    // Pass 1.0 to force exact pixel size.
    UIGraphicsBeginImageContextWithOptions(newSize, NO, 1.0);
    [image drawInRect: CGRectMake(0, 0, newSize.width, newSize.height)];
    UIImage *newImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();

    return newImage;
}

@end

//@implementation SocialNetworkFlowLayout
//
//#define SOCIAL_NETWORK_ITEM_SPACING 0.0
//#define SOCIAL_NETWORK_ITEM_HEIGHT 80.0
//
//- (instancetype) init
//{
//    if (self = [super init]) {
//        self.minimumLineSpacing = SOCIAL_NETWORK_ITEM_SPACING;
//        self.minimumInteritemSpacing = SOCIAL_NETWORK_ITEM_SPACING;
//        self.scrollDirection = UICollectionViewScrollDirectionVertical;
//    }
//
//    return self;
//}
//
//- (CGSize) itemSize
//{
////    NSInteger numberOfColumns = 3;
////
////    CGFloat itemWidth = (CGRectGetWidth(self.collectionView.frame) - (numberOfColumns - SOCIAL_NETWORK_ITEM_SPACING)) / numberOfColumns;
//
////    return CGSizeMake(76.0, SOCIAL_NETWORK_ITEM_HEIGHT);
////    CGRect screenBounds = [UIScreen mainScreen].bounds;
////
////    return CGSizeMake(floor((screenBounds.size.width - 16.0) / 7.0), 44.0);
//    return CGSizeMake(88.0, 88.0);
//}
//
//@end
