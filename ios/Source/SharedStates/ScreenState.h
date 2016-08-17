//
//  ScreenState.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "TimeoutState.h"

@interface ScreenState : TimeoutState

@property (nonatomic, strong) NSString *xibName;

@property (nonatomic, weak) IBOutlet UIView *contentView;

// optional
@property (nonatomic, weak) IBOutlet UILabel *titleLabel;
@property (nonatomic, weak) IBOutlet UICollectionView *collectionView;

//@property (nonatomic, weak) IBOutlet UIView *qrCodeView;

@property (nonatomic, weak) IBOutlet UIView *topBarView;

//@property (nonatomic, weak) IBOutlet UILabel *hostnameLabel;
//@property (nonatomic, weak) IBOutlet UILabel *serverKeyLabel;
//
//@property (nonatomic, weak) IBOutlet UILabel *taxLabel;
//@property (nonatomic, weak) IBOutlet UILabel *taxRateLabel;
//@property (nonatomic, weak) IBOutlet UILabel *subtotalLabel;
//@property (nonatomic, weak) IBOutlet UILabel *totalLabel;
//
//@property (nonatomic, weak) IBOutlet UILabel *publicKeyLabel;
@property (nonatomic, weak) IBOutlet UILabel *nameLabel;
//@property (nonatomic, weak) IBOutlet UILabel *deviceTypeLabel;
//@property (nonatomic, weak) IBOutlet UILabel *appTypeLabel;
//@property (nonatomic, weak) IBOutlet UILabel *indexLabel;

@property (nonatomic, weak) IBOutlet UILabel *messageLabel;
//@property (nonatomic, weak) IBOutlet UITextField *textField;

@property (nonatomic, weak) IBOutlet UIButton *button1;
//@property (nonatomic, weak) IBOutlet UIButton *button2;
//@property (nonatomic, weak) IBOutlet UIButton *button3;
//@property (nonatomic, weak) IBOutlet UIButton *cancelButton;

@property (nonatomic, weak) IBOutlet UIImageView *imageView;

@property (nonatomic, weak) IBOutlet UIWebView *webView;

- (void) styleButton: (UIButton *) button;

@end
