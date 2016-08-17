//
//  BlueBioController.m
//  Follow
//
//  Created by Erich Ocean on 7/31/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueBioController.h"

#import "BlueCardEditController.h"

#import "BlueApp.h"

@interface BlueBioController () <UITextViewDelegate>

@end

@implementation BlueBioController

- (void) viewDidLoad
{
    [super viewDidLoad];

    self.bioTextView.delegate = self;

    if (APP_DELEGATE.bio) {
        self.bioTextView.text = APP_DELEGATE.bio;
    }

    [self.bioTextView becomeFirstResponder];
}

#pragma mark UITextViewDelegate

- (void) textViewDidEndEditing: (UITextView *) textView
{
    APP_DELEGATE.bio = textView.text;

    [[NSNotificationCenter defaultCenter] postNotificationName: @"BioDidChange" object: nil];
}

@end
