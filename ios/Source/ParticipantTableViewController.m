//
//  ParticipantTableViewController.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "ParticipantTableViewController.h"

@implementation ParticipantTableViewController

- (void) viewWillAppear: (BOOL) animated
{
    [super viewWillAppear: animated];

    UIBarButtonItem *cancelItem = [[UIBarButtonItem alloc] initWithTitle: @"Cancel" style: UIBarButtonItemStylePlain target: self action: @selector(handleCancelTap)];

    self.navigationItem.leftBarButtonItem = cancelItem;
}

- (void) handleCancelTap
{
    [self.navigationController dismissViewControllerAnimated: YES completion: nil];
}

@end
