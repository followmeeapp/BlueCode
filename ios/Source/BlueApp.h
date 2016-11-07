//
//  BlueApp.h
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#define APP_DELEGATE ((BlueApp *)[[UIApplication sharedApplication] delegate])

@class BlueClient;
@class BlueModel;
@class BlueBackup;
@class CardObject;

#import "BlueAnalytics.h"

@interface BlueApp : UIResponder <UIApplicationDelegate>

@property (nonatomic, strong) UIWindow *window;
@property (nonatomic, strong) UIWindow *bluetoothMonitorWindow;
@property (nonatomic, strong) UIWindow *overlayWindow;

@property (nonatomic, strong) BlueClient *blueClient;
@property (nonatomic, strong) BlueModel *blueModel;
@property (nonatomic, strong) BlueBackup *blueBackup;
@property (nonatomic, strong) BlueAnalytics *blueAnalytics;

// Used by BlueSyncController. Note: These are file URLs!
@property (nonatomic, readonly) NSURL *localAvatarURL;
@property (nonatomic, readonly) NSURL *localOriginalAvatarURL;
@property (nonatomic, readonly) NSURL *localCroppedAvatarURL;
@property (nonatomic, readonly) NSURL *localCoverPhotoURL;
@property (nonatomic, readonly) NSURL *localOriginalCoverPhotoURL;
@property (nonatomic, readonly) NSURL *localCroppedCoverPhotoURL;
@property (nonatomic, readonly) NSURL *applicationDocumentsDirectory;

// Used by BlueCardEditController
@property (nonatomic, assign) BOOL isUpdatingCard;
@property (nonatomic, strong) CardObject *card;

@property (nonatomic, strong) UIImage *savedAvatar;
@property (nonatomic, strong) UIImage *originalAvatar;
@property (nonatomic, strong) UIImage *croppedAvatar;
@property (nonatomic, assign) CGRect croppedAvatarRect;
@property (nonatomic, assign) NSInteger croppedAvatarAngle;
@property (nonatomic, assign) BOOL didUpdateAvatar;

@property (nonatomic, strong) UIImage *savedCoverPhoto;
@property (nonatomic, strong) UIImage *originalCoverPhoto;
@property (nonatomic, strong) UIImage *croppedCoverPhoto;
@property (nonatomic, assign) CGRect croppedCoverPhotoRect;
@property (nonatomic, assign) NSInteger croppedCoverPhotoAngle;
@property (nonatomic, assign) BOOL didUpdateCoverPhoto;

@property (nonatomic, strong) NSString *fullName;
@property (nonatomic, strong) NSString *location;
@property (nonatomic, strong) NSString *bio;

@property (nonatomic, assign) CGFloat backgroundOpacity;
@property (nonatomic, assign) NSInteger rowOffset;

@property (nonatomic, strong) NSMutableDictionary *networks;

- (void) startBluetoothAdvertising;
- (void) startBluetoothDiscovery;

- (void) stopBluetoothAdvertising;
- (void) stopBluetoothDiscovery;

- (void) showBluetoothMonitor;
- (void) hideBluetoothMonitor;

- (void) startBlueBackup;

- (UIImage *) captureScreen;

- (void) showMain;
- (void) showMainAndCreateCard;

- (void) prepareToEditUserCard;
- (void) saveTemporaryCardProperties;

@end

@interface BlueApp (Colors)

@property (readonly) UIColor *grey;
@property (readonly) UIColor *logoBlue;
@property (readonly) UIColor *blue1;
@property (readonly) UIColor *blue2;
@property (readonly) UIColor *blue3;
@property (readonly) UIColor *blue4;
@property (readonly) UIColor *blue5;
@property (readonly) UIColor *black;
@property (readonly) UIColor *navBarColor;

@property (readonly) UIColor *bluetoothBlue;

@end

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
UIAlertView *AlertWithMessage(NSString *message);
UIAlertView *ValidationAlertWithMessage(NSString *message);
#pragma clang diagnostic pop
