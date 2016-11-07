//
//  BlueNetworkEditController.h
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface BlueNetworkEditController : UIViewController

@property (nonatomic, assign) NSInteger cardId;

@property (nonatomic, strong) NSNumber *networkKey;
@property (nonatomic, strong) NSString *networkName; // for deep linking
@property (nonatomic, strong) NSString *networkSlug; // for help URL

@property (nonatomic, copy) NSString *promptText;
@property (nonatomic, strong) UIColor *backgroundColor;
@property (nonatomic, strong) UIImage *image;

@property (nonatomic, weak) IBOutlet UIImageView *imageView;
@property (nonatomic, weak) IBOutlet UITextField *textField;
@property (nonatomic, weak) IBOutlet UIButton *button;
@property (nonatomic, weak) IBOutlet UIView *divider;

@end
