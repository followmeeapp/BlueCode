//
//  BluePrivacyController.m
//  Follow
//
//  Created by Erich Ocean on 9/5/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BluePrivacyController.h"

@interface BluePrivacyController ()

@property (nonatomic, weak) IBOutlet UITextView *textView;

@end

@implementation BluePrivacyController

- (void) viewDidLoad
{
    [super viewDidLoad];

    NSURL *url = [[NSBundle mainBundle] URLForResource: @"privacy-policy" withExtension: @"rtf"];
    NSAttributedString *text = [[NSAttributedString alloc] initWithURL:        url
                                                           options:            @{ NSDocumentTypeDocumentAttribute: NSRTFTextDocumentType }
                                                           documentAttributes: nil
                                                           error:              nil                                                       ];

    self.textView.attributedText = text;
    [self.textView scrollRangeToVisible: NSMakeRange(0, 10)];
}

@end
