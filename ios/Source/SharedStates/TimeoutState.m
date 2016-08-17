//
//  TimeoutState.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "TimeoutState.h"

#import "AppController.h"

@interface TimeoutState ()

@property (nonatomic, strong) NSTimer *timer;

@end

@implementation TimeoutState

- (instancetype) init
{
    if (self = [super init]) {
        _timeout = 0.0;
    }

    return self;
}

- (void) enterState
{
    if (_timeout > 0) {
        _timer = [NSTimer timerWithTimeInterval: _timeout
                          target:                self
                          selector:              @selector(timeoutDispatch:)
                          userInfo:              nil
                          repeats:               NO                        ];

        [[NSRunLoop currentRunLoop] addTimer: _timer forMode: NSRunLoopCommonModes];
    }
}

- (void) exitState
{
    [_timer invalidate];
    _timer = nil;
}

#pragma mark Actions

- (void) timeout: (NSTimer *) timer
{
    [self showNotImplementedAlert: _cmd];
}

#pragma mark Helpers

- (void) timeoutDispatch: (NSTimer *) timer
{
    [APP_CONTROLLER dispatchTimeout: timer];
}

- (void) showNotImplementedAlert: (SEL) selector
{
    NSString *title = [NSString stringWithFormat: @"%@ is not implemented.", NSStringFromSelector(selector)];

    [self showAlert: title];
}

- (void) showAlert: (NSString *) title
{
    NSAssert([[title class] isSubclassOfClass: [NSString class]], @"title argument must be an NSString");

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     title
                                                  message:           @""
                                                  delegate:          nil
                                                  cancelButtonTitle: @"OK"
                                                  otherButtonTitles: nil  ];
#pragma clang diagnostic pop

    [alertView show];
}

@end
