//
//  BlueSyncController.m
//  Follow
//
//  Created by Erich Ocean on 10/10/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueSyncController.h"

#import "BlueApp.h"
#import "BlueModel.h"
#import "BlueClient.h"

#import "UserObject.h"
#import "CardObject.h"
#import "NetworkObject.h"
#import "DeviceObject.h"
#import "SectionObject.h"
#import "BlockedCardObject.h"

#import "ServerRequest.h"

@interface BlueSyncController ()

@end

@implementation BlueSyncController

- (void) viewDidLoad
{
    [super viewDidLoad];

//    if (card.avatarURLString) {
//        // FIXME(Erich): This only works with internet access! And it blocks the UI. Use an async
//        // download method with a timeout, and call the callback.
//        self.savedAvatar = [UIImage imageWithData: [NSData dataWithContentsOfURL: [NSURL URLWithString: card.avatarURLString]]];
//
//    } else {
//        self.savedAvatar = nil;
//    }
//
//    if (card.backgroundURLString) {
//        // FIXME(Erich): This only works with internet access! And it blocks the UI. Use an async
//        // download method with a timeout, and call the callback.
//        self.savedCoverPhoto = [UIImage imageWithData: [NSData dataWithContentsOfURL: [NSURL URLWithString: card.backgroundURLString]]];
//
//    } else {
//        self.savedCoverPhoto = nil;
//    }
}

@end
