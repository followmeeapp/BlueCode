//
//  BlueTOSController.m
//  Follow
//
//  Created by Erich Ocean on 9/5/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueTOSController.h"

@interface BlueTOSController ()

@property (nonatomic, weak) IBOutlet UITextView *textView;

@end

@implementation BlueTOSController

- (void) viewDidLoad
{
    [super viewDidLoad];

    NSURL *url = [[NSBundle mainBundle] URLForResource: @"terms-of-use" withExtension: @"rtf"];
    NSAttributedString *text = [[NSAttributedString alloc] initWithURL:        url
                                                           options:            @{ NSDocumentTypeDocumentAttribute: NSRTFTextDocumentType }
                                                           documentAttributes: nil
                                                           error:              nil                                                       ];

    self.textView.attributedText = text;
    [self.textView scrollRangeToVisible: NSMakeRange(0, 10)];
}

@end
