//
//  IntroController.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "IntroController.h"

#import "Follow.h"

#import <DigitsKit/DigitsKit.h>

@interface IntroController ()

@property (weak, nonatomic) IBOutlet UIButton *button;

@end

@implementation IntroController

- (void) viewDidLoad
{
    [super viewDidLoad];

    UIButton *button = self.button;
    button.layer.cornerRadius = 5.0;
    button.layer.borderWidth = 1.0;
    button.layer.borderColor = [UIColor whiteColor].CGColor;
}

- (BOOL) prefersStatusBarHidden
{
    return YES;
}

- (void) didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

- (IBAction) login: sender
{
    DGTAppearance *appearance = [[DGTAppearance alloc] init];
    appearance.backgroundColor = APP_DELEGATE.blue5;
    appearance.accentColor = [UIColor whiteColor];

    DGTAuthenticationConfiguration *configuration = [[DGTAuthenticationConfiguration alloc] initWithAccountFields: DGTAccountFieldsDefaultOptionMask];
    configuration.appearance = appearance;

    [[Digits sharedInstance] authenticateWithViewController: nil configuration: configuration completion: ^(DGTSession *session, NSError *error) {
        if (session) {
            // Delay showing the contacts uploader while the Digits screen animates off-screen
//            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
//                [self uploadDigitsContactsForSession: session];
//            });

            dispatch_async(dispatch_get_main_queue(), ^{
                NSString *emailAddress = session.emailAddress;

                [self performSegueWithIdentifier: @"DidAuthenticateFirstTime" sender: self];
//                [APP_CONTROLLER sendAction: @"digitsSessionDidAuthenticate" withArguments: @{
//                    @"digits": @{
//                        @"userID": session.userID,
//                        @"emailAddress": emailAddress? emailAddress : [NSNull null],
//                        @"emailAddressIsVerified": session.emailAddressIsVerified ? @YES : @NO,
//                        @"phoneNumber": session.phoneNumber,
//                    }
//                }];

//                NSString *msg = [NSString stringWithFormat:@"Email address: %@, phone number: %@", session.emailAddress, session.phoneNumber];
//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wdeprecated-declarations"
//                UIAlertView *alert = [[UIAlertView alloc] initWithTitle:     @"You are logged in!"
//                                                          message:           msg
//                                                          delegate:          nil
//                                                          cancelButtonTitle: @"OK"
//                                                          otherButtonTitles: nil                 ];
//                [alert show];
//#pragma clang diagnostic pop
            });

        } else {
            NSLog(@"Authentication error: %@", error.localizedDescription);
//            [APP_CONTROLLER sendAction: @"digitsSessionDidError" withArguments: @{ @"error": error.localizedDescription }];
        }
    }];
}

@end
