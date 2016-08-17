//
//  AppClient.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "AppClient.h"

#import "AppController.h"

@implementation AppClient

- (void) webSocketDidReceiveMessage: (NSDictionary *) dict
{
    NSNumber *screenState = dict[@"screenState"];
    NSDictionary *data = dict[@"data"];
    NSDictionary *error = dict[@"error"];
    NSDictionary *configuration = dict[@"configuration"];
    NSDictionary *command = dict[@"command"];

    if      (screenState)   [self handleScreenStateUpdate: [screenState integerValue]];
    else if (data)          [self handleQuery: data];
    else if (error)         [self handleQueryError: error];
    else if (configuration) [self handleConfiguration: configuration];
    else if (command)       [self handleCommand: command];
    else {
        NSLog(@"Unknown message type: %@", dict);
    }
}

- (void) handleScreenStateUpdate: (NSInteger) screenState
{
    [APP_CONTROLLER dispatchScreenStateUpdate: screenState];
}

- (void) handleQuery: (NSDictionary *) data
{
    [APP_CONTROLLER handleQuery: data];
}

- (void) handleQueryError: (NSDictionary *) error
{
    [APP_CONTROLLER handleQueryError: error];
}

- (void) handleConfiguration: (NSDictionary *) configuration
{
    [APP_CONTROLLER handleConfiguration: configuration];
}

- (void) handleCommand: (NSDictionary *) command
{
    [APP_CONTROLLER handleCommand: command];
}

@end
