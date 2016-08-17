//
//  4.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Shared.h"
#import "Follow-private.h"
#define STATE_NAME 4

#import <MessageUI/MessageUI.h>

#import <TwitterKit/TwitterKit.h>

#import <FBSDKCoreKit/FBSDKCoreKit.h>
#import <FBSDKLoginKit/FBSDKLoginKit.h>

#import "ActivateCell.h"
#import "FollowCell.h"
#import "GenerateLinkBottomCell.h"

#define ACTIVATE_CELL @"ActivateCell"
#define FOLLOW_CELL @"FollowCell"
#define BOTTOM_CELL @"GenerateLinkBottomCell"

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
    
    [collectionView registerClass: [ActivateCell class] forCellWithReuseIdentifier: ACTIVATE_CELL];
    [collectionView registerClass: [FollowCell class] forCellWithReuseIdentifier: FOLLOW_CELL];
    [collectionView registerClass: [GenerateLinkBottomCell class] forCellWithReuseIdentifier: BOTTOM_CELL];
    
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
    self.digits = props[@"digits"];
    self.branchLink = props[@"branchLink"];
    self.networks = props[@"networks"];

    [self.collectionView reloadData];
    [self updateTopPanel];
}

#pragma mark - UICollectionViewDataSource delegate

- (NSInteger) numberOfSectionsInCollectionView: (UICollectionView *) collectionView
{
    return 2;
}

- (NSInteger)
collectionView:         (UICollectionView *) collectionView
numberOfItemsInSection: (NSInteger)          section
{
    assert(section < 2 && "Section should be less than two");
    
    if (section == 0) {
        return self.networks.count;
        
    } else {
        return 1;
    }
}

- (UICollectionViewCell *)
collectionView:         (UICollectionView *) collectionView
cellForItemAtIndexPath: (NSIndexPath *)      indexPath
{
    NSInteger section = [indexPath section];
    assert(section < 2 && "Section should be less than two");

    if (section == 0) {
        // Now we need to configure the cell.
        NSDictionary *network = self.networks[[indexPath row]];
        NSString *user = network[@"user"];

        if (user && [[user class] isSubclassOfClass: [NSString class]]) {
            FollowCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier: FOLLOW_CELL forIndexPath: indexPath];

            cell.imageView.image = [UIImage imageNamed: network[@"icon"]];
            cell.networkLabel.text = network[@"name"];
            cell.followeeLabel.text = user;
            if (cell.toggle.on != [network[@"enabled"] boolValue]) cell.toggle.on = [network[@"enabled"] boolValue];
            cell.divider.alpha = [indexPath row] == (self.networks.count-1) ? 0.0 : 1.0;
            cell.toggle.tag = [indexPath row];

            return cell;

        } else {
            ActivateCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier: ACTIVATE_CELL forIndexPath: indexPath];

            cell.imageView.image = [UIImage imageNamed: network[@"icon"]];
            cell.divider.alpha = [indexPath row] == (self.networks.count-1) ? 0.0 : 1.0;

            cell.button.layer.borderColor = [UIColor colorWithRed: 41.0/255.0 green: 160.0/255.0 blue: 247.0/255.0 alpha: 1.0].CGColor;
            cell.button.layer.borderWidth = 0.5;
            cell.button.layer.cornerRadius = 8.0;
            cell.button.tag = [indexPath row];

            return cell;
        }

    } else {
        GenerateLinkBottomCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier: BOTTOM_CELL forIndexPath: indexPath];

        // Now we need to configure the cell.
        [self styleButton: cell.button];

        return cell;
    }
}

- (CGSize)
collectionView:         (UICollectionView *)       collectionView
layout:                 (UICollectionViewLayout *) collectionViewLayout
sizeForItemAtIndexPath: (NSIndexPath *)            indexPath
{
    NSInteger section = [indexPath section];
    assert(section < 2 && "Section should be less than two");
    
    if (section == 0) {
        return CGSizeMake(collectionView.frame.size.width, 80);
        
    } else {
        return CGSizeMake(collectionView.frame.size.width, 197);
    }
}

#pragma mark - Actions

- (IBAction) tappedSwitch: (UISwitch *) sender
{
    [self handled];
    
//    BOOL isBecomingOn = [sender isOn];

    // FIXME: This depends on the network array ordering!
    switch ([sender tag]) {
        case 0: [APP_CONTROLLER sendAction: @"toggleTwitterEnabled"]; break;
        case 1: [APP_CONTROLLER sendAction: @"toggleFacebookEnabled"]; break;
        case 2: [APP_CONTROLLER sendAction: @"toggleInstagramEnabled"]; break;
    }
}

- (IBAction) tappedButton: (UIButton *) sender
{
    NSDictionary *network = self.networks[sender.tag];
    NSString *name = network[@"name"];

    if ([name isEqualToString: @"Twitter"]) {
        [self handled];

        [[Twitter sharedInstance] logInWithCompletion: ^(TWTRSession *session, NSError *error) {
            if (session) {
                [APP_CONTROLLER sendAction: @"loadTwitterSession" withArguments: @{ @"twitterUserID": [session userID] }];
                
            } else {
                [APP_CONTROLLER sendAction: @"loadTwitterSessionError"];
            }
        }];

    } else if ([name isEqualToString: @"Facebook"]) {
        [self handled];

        FBSDKLoginManager *login = [[FBSDKLoginManager alloc] init];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        [login logInWithReadPermissions: @[@"public_profile"] handler: ^(FBSDKLoginManagerLoginResult *result, NSError *error) {
            if (error) {
                // Process error
                NSLog(@"Facebook login error %@",error);
                [APP_CONTROLLER sendAction: @"loadFacebookSessionError"];

            } else if (result.isCancelled) {
                NSLog(@"Facebook login cancelled");

            } else {
                if ([result.grantedPermissions containsObject: @"public_profile"]) {
                    NSLog(@"%@", result);
                    [APP_CONTROLLER sendAction: @"loadFacebookSession" withArguments: @{ @"facebookUserID": result.token.userID }];

                } else {
                    [APP_CONTROLLER sendAction: @"loadFacebookSessionError"];
                }
            }
        }];
#pragma clang diagnostic pop

    } else if ([name isEqualToString: @"Instagram"]) {
        [self handled];

        [APP_CONTROLLER sendAction: @"addInstagram"];
    }
}

- (IBAction) generateLink: sender
{
    [self handled];

    MFMessageComposeViewController *mc = [[MFMessageComposeViewController alloc] init];
    mc.messageComposeDelegate = (id<MFMessageComposeViewControllerDelegate>)self;
    mc.body = [NSString stringWithFormat: @"Check me out on Follow!\n%@", self.branchLink];

    // Present message view controller on screen
    [UIView setAnimationsEnabled: YES];
    [APP_CONTROLLER.window.rootViewController presentViewController: mc animated: YES completion: NULL];
}

#pragma mark - MFMessageComposeViewControllerDelegate

- (void)
messageComposeViewController: (MFMessageComposeViewController *) controller
didFinishWithResult:          (MessageComposeResult)             result
{
    // Close the Message Interface
    [UIView setAnimationsEnabled: YES];
    [APP_CONTROLLER.window.rootViewController dismissViewControllerAnimated: YES completion: NULL];
    
    if (result == MessageComposeResultSent) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1), dispatch_get_main_queue(), ^(void) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
//            UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     @"Your crew now knows\nwhat's up."
//                                                          message:           @"Thank you for making RR\na better place to chill. :)"
//                                                          delegate:          self.controller.delegate
//                                                          cancelButtonTitle: @"OK"
//                                                          otherButtonTitles: nil ];
//            [UIView setAnimationsEnabled: YES];
//            [alertView show];
#pragma clang diagnostic pop
        });
    }
}

@end
