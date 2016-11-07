//
//  BlueCopyrightController.m
//  Follow
//
//  Created by Erich Ocean on 11/3/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueCopyrightController.h"

@interface BlueCopyrightController ()

@property (nonatomic, weak) IBOutlet UITextView *textView;

@end

@implementation BlueCopyrightController

- (void) viewDidLoad
{
    [super viewDidLoad];

    NSURL *url = [[NSBundle mainBundle] URLForResource: @"copyright-policy" withExtension: @"rtf"];
    NSAttributedString *text = [[NSAttributedString alloc] initWithURL:        url
                                                           options:            @{ NSDocumentTypeDocumentAttribute: NSRTFTextDocumentType }
                                                           documentAttributes: nil
                                                           error:              nil                                                       ];

    self.textView.attributedText = text;
    [self.textView scrollRangeToVisible: NSMakeRange(0, 10)];
}

@end
