//
//  BlueBluetoothPermissionController.m
//  Follow
//
//  Created by Erich Ocean on 8/16/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueBluetoothPermissionController.h"

#import "BlueApp.h"

@implementation BlueBluetoothPermissionController

- (IBAction) gotIt: sender
{
    [APP_DELEGATE startBluetoothAdvertising];
    [APP_DELEGATE startBluetoothDiscovery];

    [self.navigationController popToRootViewControllerAnimated: YES];
}

@end
