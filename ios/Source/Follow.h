//
//  Follow.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//
//  Some code in this project is derived from Atlas Messenger:
//  https://github.com/layerhq/Atlas-Messenger-iOS
//  Copyright (c) 2015 Layer, Inc. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//


#import <LayerKit/LayerKit.h>

#import "LayerClient.h"
#import "ServerClient.h"

#define APP_DELEGATE ((Follow *)[[UIApplication sharedApplication] delegate])

extern NSString *const ConversationMetadataDidChangeNotification;
extern NSString *const ConversationParticipantsDidChangeNotification;
extern NSString *const ConversationDeletedNotification;


@interface Follow : UIResponder <UIApplicationDelegate, LYRClientDelegate>

@property (nonatomic, strong) UIWindow *window;

@property (nonatomic, strong) LayerClient *layerClient;
@property (nonatomic, strong) ServerClient *serverClient;

@property (readonly) UIColor *blue1;
@property (readonly) UIColor *blue2;
@property (readonly) UIColor *blue3;
@property (readonly) UIColor *blue4;
@property (readonly) UIColor *blue5;
@property (readonly) UIColor *black;

@end

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
UIAlertView *AlertWithError(NSError *error);
#pragma clang diagnostic pop
