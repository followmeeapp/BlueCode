//
//  BlueClient.h
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "SRWebSocket.h"

@interface BlueClient : NSObject <SRWebSocketDelegate>

@property (nonatomic, assign) BOOL hasWebSocket;

@property (nonatomic, copy) NSData *publicKey;
@property (nonatomic, copy) NSData *serverKey;

- (instancetype)
initWithServerKey: (NSData *)   serverKey
hostname:          (NSString *) hostname
port:              (NSInteger)  port;

- (void) stopWebSocket;
- (void) startWebSocket;

- (void) sendRequest: (NSData *) data;

//- (void) webSocketDidReceiveMessage: (NSDictionary *) dict;

- (BOOL) shouldPing;

- (NSInteger) nextRequestId;

@end
