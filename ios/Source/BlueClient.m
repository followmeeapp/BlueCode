//
//  BlueClient.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueClient.h"

#import "BlueApp.h"
#import "Utilities.h"

#import "ServerRequest.h"
#import "ServerResponse.h"

#import "NetworkObject.h"

#include "tweetnacl.h"
#include "randombytes.h"

@interface BlueClient ()

@property (nonatomic, strong) SRWebSocket *webSocket;

@property (nonatomic, assign) BOOL didStartWebSocket;

@property (nonatomic, copy) NSData *secretKey;

@property (nonatomic, copy) NSString *hostname;
@property (nonatomic, assign) NSInteger port;

@property (nonatomic, strong) NSTimer *timer;

@property (nonatomic, strong) NSMutableArray *pendingMessages;

@property (nonatomic, assign) NSInteger lastRequestId;

- (NSInteger) nextRequestId;

@end

@implementation BlueClient

- (instancetype)
initWithServerKey: (NSData *)   serverKey
hostname:          (NSString *) hostname
port:              (NSInteger)  port
{
    if (self = [super init]) {
        self.serverKey = serverKey;
        self.hostname = hostname;
        _port = port;

        self.pendingMessages = [NSMutableArray arrayWithCapacity: 10];

        _lastRequestId = 0;

        [self startWebSocket];
    }

    return self;
}

- (void) dealloc
{
    [self.timer invalidate];
    self.timer = nil;
}

- (void) createKeyPair
{
    // We generate a new keypair for each connection.
    unsigned char publicKeyBytes[crypto_box_PUBLICKEYBYTES];
    unsigned char secretKeyBytes[crypto_box_SECRETKEYBYTES];

    crypto_box_keypair(publicKeyBytes, secretKeyBytes);

    NSData *publicKey = [NSData dataWithBytes: publicKeyBytes length: crypto_box_PUBLICKEYBYTES];
    NSData *secretKey = [NSData dataWithBytes: secretKeyBytes length: crypto_box_SECRETKEYBYTES];

    self.publicKey = publicKey;
    self.secretKey = secretKey;
}

- (void) stopWebSocket
{
    [self.timer invalidate];
    self.timer = nil;

    [_webSocket close];
    _webSocket = nil;

    _hasWebSocket = NO;
    _didStartWebSocket = NO;
}

- (BOOL) shouldPing { return NO; }

- (void) reopenWebSocketIfNeeded: (NSTimer *) timer
{
//    NSLog(@"-[BlueClient reopenWebSocketIfNeeded:]");

    if (_webSocket) {
//        if (self.shouldPing && _webSocket.readyState == SR_OPEN) [self sendRequest: @"ping"];
        return;
    }

    [self createKeyPair];

    NSString *urlString = [NSString stringWithFormat: @"ws://%@:%@/", self.hostname, @(self.port)];
    NSURL *url = [NSURL URLWithString: urlString];
    NSLog(@"WebSocket URL: %@", url);

//    NSDictionary *cookieProperties = [NSDictionary dictionaryWithObjectsAndKeys:
//                                          self.hostname,              NSHTTPCookieDomain,
//                                          @"\\",                      NSHTTPCookiePath,
//                                          @"publickey",               NSHTTPCookieName,
//                                          [self.publicKey hexString], NSHTTPCookieValue, nil];
//
//    NSHTTPCookie *cookie = [NSHTTPCookie cookieWithProperties: cookieProperties];
//    NSArray *cookieArray = [NSArray arrayWithObject: cookie];
//    NSDictionary *headers = [NSHTTPCookie requestHeaderFieldsWithCookies: cookieArray];

    NSMutableURLRequest *urlRequest = [NSMutableURLRequest requestWithURL: url];
//    [urlRequest setAllHTTPHeaderFields: headers];

    _webSocket = [[SRWebSocket alloc] initWithURLRequest: urlRequest];
    _webSocket.delegate = self;

    [_webSocket open];
}

- (void) startWebSocket
{
    if (_didStartWebSocket) return;

    _didStartWebSocket = YES;

    _hasWebSocket = NO; // This is only YES when we can actually *use* the WebSocket.

    [self reopenWebSocketIfNeeded: nil];

    self.timer = [NSTimer timerWithTimeInterval: 1.0
                          target:                self
                          selector:              @selector(reopenWebSocketIfNeeded:)
                          userInfo:              nil
                          repeats:               YES                               ];

    [[NSRunLoop currentRunLoop] addTimer: self.timer forMode: NSRunLoopCommonModes];
}

#pragma mark SRWebSocketDelegate

- (void) webSocketDidOpen: (SRWebSocket *) webSocket
{
//    NSLog(@"-[BlueClient webSocketDidOpen:]");
    _hasWebSocket = YES;

//    [self webSocketStatusDidChange: _hasWebSocket];

    NSData *helloRequestPacket = [ServerRequest newHelloRequestWithId: [self nextRequestId]];
    if (_webSocket && _webSocket.readyState == SR_OPEN) {
        [_webSocket send: helloRequestPacket];

    } else {
        [_webSocket close];
        return;
    }

    [self.pendingMessages enumerateObjectsUsingBlock: ^(NSData *packet, NSUInteger idx, BOOL *stop) {
        if (_webSocket && _webSocket.readyState == SR_OPEN) {
            [_webSocket send: packet];
        }
    }];

    [self.pendingMessages removeAllObjects];
}

- (NSInteger) nextRequestId
{
    return ++_lastRequestId;
}

- (void)
webSocket:        (SRWebSocket *) webSocket
didFailWithError: (NSError *)     error
{
    NSLog(@"-[BlueClient webSocket:didFailWithError:] %@", error);
    _hasWebSocket = NO;

//    [self webSocketStatusDidChange: _hasWebSocket];
    _webSocket = nil;
}

// message will either be an NSString if the server is using text
// or NSData if the server is using binary.
- (void)
webSocket:         (SRWebSocket *) webSocket
didReceiveMessage: (NSData *)      message
{
//    NSLog(@"-[BlueClient webSocket:didReceiveMessage:]");
//    DDLogInfo(@"Received %@", message);

    if (![message isKindOfClass: [NSData class]]) {
        NSLog(@"We only support data.");
        return;
    }

    if (!self.serverKey) {
        // No crypto is being used.
        [ServerResponse handleResponse: message];

    } else {
        NSData *nonce = [message subdataWithRange: NSMakeRange(0, 24)];
        NSData *cipherText = [message subdataWithRange: NSMakeRange(24, message.length - 24)];
        NSMutableData *plainText = [NSMutableData dataWithLength: cipherText.length];

        int result = crypto_box_open((unsigned char *)plainText.bytes,
                                     cipherText.bytes,
                                     cipherText.length,
                                     nonce.bytes,
                                     self.serverKey.bytes,
                                     self.secretKey.bytes);

        if (result == -1) {
            NSLog(@"Decryption failed.");
            return;
        }

        NSData *decryptedMessage = [plainText subdataWithRange: NSMakeRange(32, plainText.length - 32)];

        [ServerResponse handleResponse: decryptedMessage];
    }
}

- (void)
webSocket:      (SRWebSocket *) webSocket
didReceivePong: (NSData *)      pongPayload
{
    NSLog(@"-[BlueClient webSocket:didReceivePong:]");
}

- (void)
webSocket:        (SRWebSocket *) webSocket
didCloseWithCode: (NSInteger)     code
reason:           (NSString *)    reason
wasClean:         (BOOL)          wasClean
{
    NSLog(@"-[BlueClient webSocket:didCloseWithCode:reason:wasClean:]");
    _webSocket = nil;
    _hasWebSocket = NO;

//    [self webSocketStatusDidChange: _hasWebSocket];
}

#pragma mark WebSocket helpers

- (void) sendRequest: (NSData *) data
{
    if (!self.serverKey) {
        // No crypto is being used.
        if (_webSocket && _webSocket.readyState == SR_OPEN) {
            [_webSocket send: data];

        } else {
            [self.pendingMessages addObject: data];
        }

    } else {
        NSData *packet = [self encryptData: data];

        if (!packet) {
            NSLog(@"Something went wrong with the encryption");
            return;
        }

        if (_webSocket && _webSocket.readyState == SR_OPEN) {
            [_webSocket send: packet];

        } else {
            [self.pendingMessages addObject: packet];
        }
    }
}

- (NSData *) encryptData: (NSData *) message
{
    // Create the nonce.
    const unsigned char nonce[crypto_box_NONCEBYTES];
    randombytes((unsigned char *)nonce, crypto_box_NONCEBYTES);

    // Create a buffer to encrypt message from and into.
    NSMutableData *encryptionSource = [NSMutableData dataWithLength: message.length + 32];
    NSMutableData *encryptionDestination = [NSMutableData dataWithLength: message.length + 32];
    // NOTE: MUST be zero filled. Cocoa does this automatically. Buffers are the same size.

    // Copy in the message. Leave 32 zero bytes in the front.
    memcpy((void *)encryptionSource.bytes + 32, message.bytes, message.length);

    // Encrypt message
    crypto_box((unsigned char *)encryptionDestination.bytes,
               encryptionSource.bytes,
               encryptionSource.length,
               nonce,
               _serverKey.bytes,
               _secretKey.bytes);

    // Create the final packet.
    NSMutableData *packet = [NSMutableData dataWithLength: 24 + encryptionDestination.length];

    // Copy in the 24 random nonce bytes.
    memcpy((void *)packet.bytes, nonce, 24);

    // Copy in the encrypted message plus the 16 zero bytes at the beginning.
    memcpy((void *)packet.bytes + 24, encryptionDestination.bytes, encryptionDestination.length);

    return packet;
}

//#pragma mark Template methods
//
//- (void) webSocketStatusDidChange: (BOOL) hasWebSocket
//{
////    [APP_DELEGATE webSocketStatusDidChange: hasWebSocket];
//}
//
//- (void) webSocketDidReceiveMessage: (NSData *) data
//{
////    [APP_DELEGATE webSocketDidReceiveMessage: data];
//}

@end
