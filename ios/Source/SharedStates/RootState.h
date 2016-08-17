//
//  RootState.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "EOState.h"

@interface RootState : EOState

// action signatures for states

- (IBAction) tappedButton: sender;

- (IBAction) tappedSwitch: sender;
- (IBAction) tappedFollow: sender;

- (IBAction) startStopQRCodeReading: sender;
- (IBAction) didScanQRCode: sender;

- (IBAction) reset: sender;
- (IBAction) touchStartVideo: sender;
- (IBAction) touchStartPDF: sender;
- (IBAction) touchStartManual: sender;
- (IBAction) touchCancel: sender;
- (IBAction) touchDragExit: sender;
- (IBAction) touchDragEnter: sender;
- (IBAction) touchUpOutside: sender;
- (IBAction) touchUpInside: sender;

- (IBAction) tappedBegin: sender;
- (IBAction) tappedSizeButton: sender;
- (IBAction) tappedBuyButton: sender;

- (IBAction) tappedEnterConfirmationCode: sender;

- (IBAction) tappedTryAgain: sender;
- (IBAction) tappedShare: sender;
- (IBAction) tappedAddDevice: sender;
- (IBAction) tappedReset: sender;

- (IBAction) tappedCapture:(id)sender;

- (IBAction) tappedOK: sender;
- (IBAction) tappedYes: sender;
- (IBAction) tappedNo: sender;
- (IBAction) tappedNext: sender;
- (IBAction) tappedGetSignature: sender;
- (IBAction) tappedCancel: sender;
- (IBAction) tappedNoThanks: sender;
- (IBAction) tappedDeliverNextOrder: sender;
- (IBAction) tappedConfirm: sender;
- (IBAction) tappedContinueDelivery: sender;
- (IBAction) tappedClear: sender;
- (IBAction) tappedAccept: sender;
- (IBAction) tappedYesTakePicture: sender;
- (IBAction) tappedGotOrder: sender;
- (IBAction) tappedNumberIsCorrect: sender;
- (IBAction) tappedShutter: sender;
- (IBAction) tappedRetake: sender;
- (IBAction) tappedDone: sender;
- (IBAction) tappedLeftArrow: sender;
- (IBAction) tappedRightArrow: sender;
- (IBAction) tappedDeliverOrder: sender;
- (IBAction) tappedMap: sender;
- (IBAction) tappedOrder: sender;
- (IBAction) tappedList: sender;
- (IBAction) tappedBack: sender;
- (IBAction) tappedEnter: sender;
- (IBAction) tappedLaunchpad: sender;
- (IBAction) tappedInventory: sender;
- (IBAction) tappedPackaging: sender;
- (IBAction) tappedOrderPickUp: sender;
- (IBAction) tappedSave: sender;
- (IBAction) tappedDisableSeatDelivery: sender;
- (IBAction) tappedDisablePickUpAtGame: sender;
- (IBAction) tappedDisableNewOrders: sender;
- (IBAction) tappedStop: sender;
- (IBAction) tappedPackageNextOrder: sender;
- (IBAction) tappedContinuePackaging: sender;
- (IBAction) tappedContinue: sender;
- (IBAction) tappedEdit: sender;
- (IBAction) tappedSearch: sender;
- (IBAction) tappedFindByConfirmationCode: sender;
- (IBAction) tappedVerifyDeliveryCode: sender;
- (IBAction) incrementAvailableFromSender: sender;
- (IBAction) incrementExtraFromSender: sender;

@end
