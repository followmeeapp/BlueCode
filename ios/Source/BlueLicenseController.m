//
//  BlueLicenseController.m
//  Follow
//
//  Created by Erich Ocean on 8/31/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueLicenseController.h"

#import <AttributedMarkdown/markdown_lib.h>
#import <AttributedMarkdown/markdown_peg.h>

@interface BlueLicenseController ()

@property (nonatomic, weak) IBOutlet UITextView *textView;

@end

@implementation BlueLicenseController

- (void) viewDidLoad
{
    [super viewDidLoad];

    NSURL *url = [[NSBundle mainBundle] URLForResource: @"license" withExtension: @"txt"];
    NSString *rawText = [NSString stringWithContentsOfURL: url
                                  encoding:                NSUTF8StringEncoding
                                  error:                   NULL                ];

    // Create a font attribute for emphasized text.
    UIFont *emFont = [UIFont fontWithName: @"AvenirNext-MediumItalic" size: 15.0];

    // Create a font attribute for H2.
    UIFont *h2Font = [UIFont fontWithName:@"AvenirNext-Bold" size: 18.0];

    // Create a color attribute for paragraph text.
    UIColor *color = [UIColor blackColor];

    // Create a dictionary to hold your custom attributes for any Markdown types.
    NSDictionary *attributes = @{
        @(EMPH): @{ NSFontAttributeName : emFont, },
        @(H2):   @{ NSFontAttributeName : h2Font, },
        @(PARA): @{ NSForegroundColorAttributeName : color, }
    };

    // Parse the markdown.
    NSAttributedString *prettyText = markdown_to_attr_string(rawText, 0, attributes);

    self.textView.attributedText = prettyText;

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self.textView scrollRangeToVisible: NSMakeRange(0, 100)];
    });
}

@end
