//
//  RootState.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Shared.h"
#import "RootState.h"

#import "AppController.h"

static const int ddLogLevel __attribute__((unused)) = LOG_LEVEL_DEBUG;

@implementation RootState

- (void) enterState
{
//    [(AppController *)[[UIApplication sharedApplication] delegate] startReachability];
}

- (EOState *) defaultSubstate
{
    return nil;
}

- (void) configureState
{

}

- (void) activateState
{

}

- (void) deactivateState
{

}

- (void) exitState
{

}

//#pragma mark App Mechanics
//
//- (void) screenStateUpdate: (NSInteger) screenState
//{
//    // What this does is, within a statechart dispatch, go to the desired screenState
//    // (which is just a normal state). To do that, we need to _find_ the state.
//    NSString *stateId = [NSString stringWithFormat: @"%@", @(screenState)];
//
//    EOState *currentState = self.statechart.currentState;
//    EOState *targetState = [self findTargetStateId: stateId inSubstates: self.substates];
//
//    if (targetState && targetState != currentState) {
//        [self gotoState: targetState];
//        return;
//
//    } else if (!targetState) {
//        NSLog(@"Failed to find target state with stateId %@", stateId);
//    }
//
//    [self handled];
//}
//
//- (EOState *)
//findTargetStateId: (NSString *)     stateId
//inSubstates:       (NSDictionary *) substates
//{
//    __block EOState *targetState = nil;
//
//    [substates enumerateKeysAndObjectsUsingBlock: ^(NSString *candidateStateId, EOState *state, BOOL *stop) {
//        if ([stateId isEqualToString: candidateStateId]) {
//            targetState = state;
//            *stop = YES;
//
//        } else {
//            targetState = [self findTargetStateId: stateId inSubstates: state.substates];
//            if (targetState) *stop = YES;
//        }
//    }];
//
//    return targetState;
//}
//
#pragma mark Actions

- (IBAction) tappedButton: sender
{
    [self showNotImplementedAlert: _cmd];
}


- (IBAction) generateLink: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedSwitch: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedFollow: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) startStopQRCodeReading: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) didScanQRCode: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) reset: sender
{
    // Don't log this.
}

- (IBAction) touchStartVideo: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) touchStartPDF: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) touchStartManual: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) touchCancel: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) touchDragExit: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) touchDragEnter: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) touchUpOutside: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) touchUpInside: sender
{
    [self showNotImplementedAlert: _cmd];
}



- (IBAction) tappedBegin: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedSizeButton: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedBuyButton: sender
{
    [self showNotImplementedAlert: _cmd];
}



- (IBAction) tappedEnterConfirmationCode: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedTryAgain: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedShare: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedAddDevice: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedReset: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedCapture:(id)sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedOK: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedYes: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedNo: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedNext: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedGetSignature: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedCancel: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedNoThanks: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedDeliverNextOrder: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedConfirm: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedContinueDelivery: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedClear: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedAccept: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedYesTakePicture: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedGotOrder: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedNumberIsCorrect: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedShutter: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedRetake: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedDone: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedLeftArrow: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedRightArrow: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedDeliverOrder: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedMap: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedOrder: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedList: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedBack: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedEnter: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedLaunchpad: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedInventory: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedPackaging: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedOrderPickUp: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedSave: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedDisableSeatDelivery: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedDisablePickUpAtGame: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedDisableNewOrders: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedStop: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedPackageNextOrder: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedContinuePackaging: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedContinue: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedEdit: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedSearch: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedFindByConfirmationCode: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) tappedVerifyDeliveryCode: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) incrementAvailableFromSender: sender
{
    [self showNotImplementedAlert: _cmd];
}

- (IBAction) incrementExtraFromSender: sender
{
    [self showNotImplementedAlert: _cmd];
}

#pragma mark Helpers

- (void) showNotImplementedAlert: (SEL) selector
{
    NSString *title = [NSString stringWithFormat: @"%@ is not implemented.", NSStringFromSelector(selector)];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:     title
                                                  message:           @""
                                                  delegate:          nil
                                                  cancelButtonTitle: @"OK"
                                                  otherButtonTitles: nil  ];
    [alertView show];
#pragma clang diagnostic pop
}

@end
