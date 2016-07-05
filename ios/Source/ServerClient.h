//
//  ServerClient.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "SRWebSocket.h"

@interface ServerClient : NSObject <SRWebSocketDelegate>

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

@end
