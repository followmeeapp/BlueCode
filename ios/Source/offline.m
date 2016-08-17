//
//  offline.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Shared.h"
#import "Follow-private.h"
#define STATE_NAME OFFLINE

static const int ddLogLevel __attribute__((unused)) = LOG_LEVEL_DEBUG;

@interface STATE : ScreenState
@end

@implementation STATE

SUBSTATES( CONNECTING, CONNECTION_FAILED, SERVER_UNREACHABLE )

- (NSString *) xibName
{
    return @"LaunchScreen";
}

- (EOState *) defaultSubstate
{
    assert(NO && "You cannot transition directly to the OFFLINE state.");

    return nil;
}

#pragma mark - Actions

- (void) webSocketStatusDidChange: (BOOL) hasWebSocket
{
    if (hasWebSocket) {
        [self handled];

    } else {
        [self gotoState: [self substateWithPath: @"CONNECTION_FAILED"]];
    }
}

- (void) internetStatusDidChange: (BOOL) hasInternet
{
    if (!hasInternet) {
        [self gotoState: [self substateWithName: @"SERVER_UNREACHABLE"]];
        
    } else {
        [self handled];
    }
}

@end
