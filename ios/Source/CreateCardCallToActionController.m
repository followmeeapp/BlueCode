//
//  CreateCardCallToActionController.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "CreateCardCallToActionController.h"

@interface CreateCardCallToActionController ()

@property (weak, nonatomic) IBOutlet UIButton *button;

@end

@implementation CreateCardCallToActionController

- (void) viewDidLoad
{
    [super viewDidLoad];

    UIButton *button = self.button;
    button.layer.cornerRadius = 5.0;
    button.layer.borderWidth = 1.0;
    button.layer.borderColor = [UIColor whiteColor].CGColor;
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

@end
