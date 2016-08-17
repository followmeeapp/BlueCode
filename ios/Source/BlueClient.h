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

- (instancetype)
initWithSecretKey: (NSData *)   secretKey
publicKey:         (NSData *)   publicKey
serverKey:         (NSData *)   serverKey
hostname:          (NSString *) hostname
port:              (NSInteger)  port;

- (void) stopWebSocket;
- (void) startWebSocket;

- (void) sendRequest: (NSData *) data;

- (void) webSocketDidReceiveMessage: (NSDictionary *) dict;

- (BOOL) shouldPing;

- (NSInteger) nextRequestId;

@end
