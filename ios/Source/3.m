//
//  3.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Shared.h"
#import "Follow-private.h"
#define STATE_NAME 3

#import "FollowCardCell.h"

#define FOLLOW_CARD_CELL @"FollowCardCell"

static const int ddLogLevel __attribute__((unused)) = LOG_LEVEL_DEBUG;

@interface STATE : ScreenState <UICollectionViewDelegate, UICollectionViewDataSource>

@property (nonatomic, strong) NSDictionary *digits;
@property (nonatomic, strong) NSString *branchLink;
@property (nonatomic, strong) NSArray *networks;

@end

@implementation STATE

- (void) enterState
{
    [super enterState];
    
    self.digits = @{};
    self.branchLink = @"";
    self.networks = @[];
    
    // Set drop shadow
    UIView *view = self.topBarView;
    view.layer.shadowColor = [[UIColor blackColor] CGColor];
    view.layer.shadowOpacity = 0.3f;
    view.layer.shadowPath = [[UIBezierPath bezierPathWithRect: view.bounds] CGPath];
    view.layer.shadowOffset = CGSizeMake(0.0, 4.0);
    view.layer.shadowRadius = 6.0f;
    
    // Configure collection view
    UICollectionView *collectionView = self.collectionView;
    
    collectionView.delegate = self;
    collectionView.dataSource = self;
    
    [collectionView registerClass: [FollowCardCell class] forCellWithReuseIdentifier: FOLLOW_CARD_CELL];
    
    [collectionView reloadData];
    [self updateTopPanel];
}

- (void) updateTopPanel
{
    NSString *phoneNumber = self.digits[@"phoneNumber"];
    
    self.nameLabel.text = phoneNumber? phoneNumber : @"";
    self.messageLabel.text = self.branchLink;
}

- (void) render: (NSDictionary *) props
{
//    self.digits = props[@"digits"];
//    self.branchLink = props[@"branchLink"];
    self.networks = props[@"cardNetworks"];
    
    [self.collectionView reloadData];
    [self updateTopPanel];
}

#pragma mark - UICollectionViewDataSource delegate

- (NSInteger) numberOfSectionsInCollectionView: (UICollectionView *) collectionView
{
    return 1;
}

- (NSInteger)
collectionView:         (UICollectionView *) collectionView
numberOfItemsInSection: (NSInteger)          section
{
    return self.networks.count;
}

- (UICollectionViewCell *)
collectionView:         (UICollectionView *) collectionView
cellForItemAtIndexPath: (NSIndexPath *)      indexPath
{
    // Now we need to configure the cell.
    NSDictionary *network = self.networks[[indexPath row]];
    NSString *user = network[@"user"];
    
    FollowCardCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier: FOLLOW_CARD_CELL forIndexPath: indexPath];

    NSDictionary *icons = @{
        @"Twitter": @"twitter-icon.png",
        @"Facebook": @"facebook-icon.png",
        @"Instagram": @"instagram-icon.png",
    };

    cell.imageView.image = [UIImage imageNamed: icons[network[@"name"]]];
    cell.networkLabel.text = network[@"name"];
    cell.followeeLabel.text = user;
    cell.divider.alpha = [indexPath row] == (self.networks.count-1) ? 0.0 : 1.0;
    cell.button.tag = [indexPath row];
    
    return cell;
}

- (CGSize)
collectionView:         (UICollectionView *)       collectionView
layout:                 (UICollectionViewLayout *) collectionViewLayout
sizeForItemAtIndexPath: (NSIndexPath *)            indexPath
{
    return CGSizeMake(collectionView.frame.size.width, 80);
}

#pragma mark - Helpers

// Expects the URL of the scheme e.g. "fb://"
- (BOOL) schemeAvailable: (NSString *) scheme
{
    return [[UIApplication sharedApplication] canOpenURL: [NSURL URLWithString: scheme]];
}

#pragma mark - Actions

- (IBAction) tappedBack: (UIButton *) sender
{
    [self handled];

    [APP_CONTROLLER sendAction: @"closeFollowCard"];
}

- (IBAction) tappedButton: (UIButton *) sender
{
    NSDictionary *network = self.networks[sender.tag];
    NSString *name = network[@"name"];

    if ([name isEqualToString: @"Twitter"]) {
        [self handled];

        BOOL installed = [self schemeAvailable: @"twitter://"];

        if (installed) {
            NSString *urlString = [NSString stringWithFormat: @"twitter://user?id=%@", network[@"userID"]];
            BOOL didOpenOtherApp = [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
            if (didOpenOtherApp) {
                NSLog(@"Opened Twitter");

            } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                UIAlertView *alert = [[UIAlertView alloc] initWithTitle:     @"Twitter not installed"
                                                          message:           nil
                                                          delegate:          nil
                                                          cancelButtonTitle: @"Drat"
                                                          otherButtonTitles: nil                      ];
                [alert show];
#pragma clang diagnostic pop
            }

        } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            UIAlertView *alert = [[UIAlertView alloc] initWithTitle:     @"Twitter not installed"
                                                      message:           nil
                                                      delegate:          nil
                                                      cancelButtonTitle: @"Drat"
                                                      otherButtonTitles: nil                      ];
            [alert show];
#pragma clang diagnostic pop
        }

    } else if ([name isEqualToString: @"Facebook"]) {
        [self handled];

        BOOL installed = [self schemeAvailable: @"fb://"];

        if (installed) {
//            NSString *urlString = [NSString stringWithFormat: @"fb://profile/%@", network[@"userID"]];
            NSString *urlString = [NSString stringWithFormat: @"https://www.facebook.com/app_scoped_user_id/%@", network[@"userID"]];
            NSLog(@"%@", urlString);
            BOOL didOpenOtherApp = [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
            if (didOpenOtherApp) {
                NSLog(@"Opened Facebook");

            } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                UIAlertView *alert = [[UIAlertView alloc] initWithTitle:     @"Facebook not installed"
                                                          message:           nil
                                                          delegate:          nil
                                                          cancelButtonTitle: @"Drat"
                                                          otherButtonTitles: nil                      ];
                [alert show];
#pragma clang diagnostic pop
            }

        } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            UIAlertView *alert = [[UIAlertView alloc] initWithTitle:     @"Facebook not installed"
                                                      message:           nil
                                                      delegate:          nil
                                                      cancelButtonTitle: @"Drat"
                                                      otherButtonTitles: nil                      ];
            [alert show];
#pragma clang diagnostic pop
        }

    } else if ([name isEqualToString: @"Instagram"]) {
        [self handled];

        BOOL installed = [self schemeAvailable: @"instagram://"];

        if (installed) {
            NSString *urlString = [NSString stringWithFormat: @"instagram://user?username=%@", network[@"userID"]];
            BOOL didOpenOtherApp = [[UIApplication sharedApplication] openURL: [NSURL URLWithString: urlString]];
            if (didOpenOtherApp) {
                NSLog(@"Opened Instagram");

            } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                UIAlertView *alert = [[UIAlertView alloc] initWithTitle:     @"Instagram not installed"
                                                          message:           nil
                                                          delegate:          nil
                                                          cancelButtonTitle: @"Drat"
                                                          otherButtonTitles: nil                      ];
                [alert show];
#pragma clang diagnostic pop
            }

        } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            UIAlertView *alert = [[UIAlertView alloc] initWithTitle:     @"Instagram not installed"
                                                      message:           nil
                                                      delegate:          nil
                                                      cancelButtonTitle: @"Drat"
                                                      otherButtonTitles: nil                      ];
            [alert show];
#pragma clang diagnostic pop
        }
    }
}

@end
