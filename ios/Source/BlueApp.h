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
@class CardObject;

@interface BlueApp : UIResponder <UIApplicationDelegate>

@property (nonatomic, strong) UIWindow *window;
@property (nonatomic, strong) UIWindow *bluetoothMonitorWindow;
@property (nonatomic, strong) UIWindow *overlayWindow;

@property (nonatomic, strong) BlueClient *blueClient;
@property (nonatomic, strong) BlueModel *blueModel;

@property (readonly) UIColor *blue1;
@property (readonly) UIColor *blue2;
@property (readonly) UIColor *blue3;
@property (readonly) UIColor *blue4;
@property (readonly) UIColor *blue5;
@property (readonly) UIColor *black;
@property (readonly) UIColor *navBarColor;

@property (readonly) UIColor *bluetoothBlue;

@property (readonly) UIColor *instinctColor;
@property (readonly) UIColor *mysticColor;
@property (readonly) UIColor *valorColor;

// Used by BlueCardEditController
@property (nonatomic, assign) BOOL isUpdatingCard;
@property (nonatomic, strong) CardObject *card;

@property (nonatomic, strong) UIImage *savedAvatar;
@property (nonatomic, strong) UIImage *originalAvatar;
@property (nonatomic, strong) UIImage *croppedAvatar;
@property (nonatomic, assign) CGRect croppedAvatarRect;
@property (nonatomic, assign) NSInteger croppedAvatarAngle;

@property (nonatomic, strong) UIImage *savedCoverPhoto;
@property (nonatomic, strong) UIImage *originalCoverPhoto;
@property (nonatomic, strong) UIImage *croppedCoverPhoto;
@property (nonatomic, assign) CGRect croppedCoverPhotoRect;
@property (nonatomic, assign) NSInteger croppedCoverPhotoAngle;

@property (nonatomic, strong) NSString *fullName;
@property (nonatomic, strong) NSString *location;
@property (nonatomic, strong) NSString *bio;

@property (nonatomic, strong) NSMutableDictionary *networks;

- (void) startBluetoothAdvertising;
- (void) startBluetoothDiscovery;

- (void) stopBluetoothAdvertising;
- (void) stopBluetoothDiscovery;

- (void) showBluetoothMonitor;
- (void) hideBluetoothMonitor;

@end

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
UIAlertView *AlertWithMessage(NSString *message);
#pragma clang diagnostic pop
