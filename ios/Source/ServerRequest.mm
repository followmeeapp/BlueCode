//
//  ServerRequest.mm
//  Follow
//
//  Created by Erich Ocean on 7/13/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "ServerRequest.h"

#import "NetworkObject.h"

#import "BlueApp.h"
#import "BlueClient.h"
#import "BlueModel.h"

#import "DeviceObject.h"
#import "CardObject.h"

#import "Utilities.h"

#include <iostream>
#include <string>

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/request.capnp.h"
#include "schema/card.capnp.h"
#include "schema/backup.capnp.h"

#import <BZipCompression/BZipCompression.h>

// FIXME(Erich): requestId should become a 16-byte value, prepended to each message,
// and used as the final 16 bytes of the encryption nonce. The first 8 bytes should
// be the deviceId (which will obviously need to be communicated to devices).
// BlueClient should create the requestId and return it after an (encrypted) send.
// This eliminates the cumbersome need to acces the requestId from BlueClient when
// creating a ServerRequest.

@implementation ServerRequest

// FIXME(Erich): Encryption and device info is a property of the connection.
// Send the UUID with initial connection request, along with publicKey info.
// Encrypt the UUID with the publicKey (and provide a nonce). Then the server
// should send back a response on the actual connect, saving a round trip and
// eliminating the need for the "Hello" packet from the client.

+ (NSData *) newHelloRequestWithId: (NSInteger) requestId
{
    capnp::MallocMessageBuilder builder;
    Request::Builder request = builder.initRoot<Request>();
    request.setId(requestId);

    NSUUID *UUID = [[UIDevice currentDevice] identifierForVendor];

    HelloRequest::Builder helloRequest = request.getKind().initHelloRequest();
    auto data = helloRequest.initUuid(16);

    uuid_t uuid;
    [UUID getUUIDBytes: uuid];

    for (int idx=0, len=16; idx<len; ++idx) {
        data[idx] = uuid[idx];
    }

    helloRequest.setVersion(4);

    if (APP_DELEGATE.blueClient.serverKey) {
        NSData *publicKey = APP_DELEGATE.blueClient.publicKey;
        char *publicKeyBytes = (char *)[publicKey bytes];

        auto pk = helloRequest.initPublicKey(32);
        for (int idx=0, len=32; idx<len; ++idx) {
            pk[idx] = publicKeyBytes[idx];
        }
    }

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    return [NSData dataWithBytes: from length: size];
}

+ (NSData *)
newJoinRequestWithId: (NSInteger)  requestId
telephone:            (NSString *) telephone
digitsId:             (NSString *) digitsId
email:                (NSString *) email
emailIsVerified:      (BOOL)       isVerified
{
    capnp::MallocMessageBuilder builder;
    Request::Builder request = builder.initRoot<Request>();
    request.setId(requestId);

    JoinRequest::Builder joinRequest = request.getKind().initJoinRequest();
    joinRequest.setTelephone([telephone UTF8String]);
    joinRequest.setDigitsId([digitsId UTF8String]);
    if (email) joinRequest.setEmail([email UTF8String]);
    joinRequest.setEmailVerified(isVerified);

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    return [NSData dataWithBytes: from length: size];
}

+ (NSData *)
newCreateCardRequestWithId: (NSInteger)      requestId
properties:                 (NSDictionary *) props
{
    capnp::MallocMessageBuilder builder;
    Request::Builder request = builder.initRoot<Request>();
    request.setId(requestId);

    CreateCardRequest::Builder cardRequest = request.getKind().initCreateCardRequest();
    Card::Builder card = cardRequest.initCard();

    if (props) {
        NSString *fullName = props[@"fullName"];
        if (fullName) card.setFullName([fullName UTF8String]);

        NSString *location = props[@"location"];
        if (location) card.setLocation([location UTF8String]);

        NSString *bio = props[@"bio"];
        if (bio) card.setBio([bio UTF8String]);

        NSString *avatarURLString = props[@"avatarURLString"];
        if (avatarURLString) card.setAvatarURLString([avatarURLString UTF8String]);

        NSString *backgroundURLString = props[@"backgroundURLString"];
        if (backgroundURLString) card.setBackgroundURLString([backgroundURLString UTF8String]);

        NSDictionary *networks = props[@"networks"];
        if (networks) {
            NSInteger count = [[networks allKeys] count];

            auto nets = card.initNetworks((unsigned int)count);
            unsigned int idx = 0;

            NSString *username = networks[@(FacebookType)];
            if (username) {
                card.setHasFacebook(true);
                nets[idx].setType(Network::Type::FACEBOOK);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(TwitterType)];
            if (username) {
                card.setHasTwitter(true);
                nets[idx].setType(Network::Type::TWITTER);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(InstagramType)];
            if (username) {
                card.setHasInstagram(true);
                nets[idx].setType(Network::Type::INSTAGRAM);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(SnapchatType)];
            if (username) {
                card.setHasSnapchat(true);
                nets[idx].setType(Network::Type::SNAPCHAT);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(GooglePlusType)];
            if (username) {
                card.setHasGooglePlus(true);
                nets[idx].setType(Network::Type::GOOGLE_PLUS);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(YouTubeType)];
            if (username) {
                card.setHasYouTube(true);
                nets[idx].setType(Network::Type::YOU_TUBE);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(PinterestType)];
            if (username) {
                card.setHasPinterest(true);
                nets[idx].setType(Network::Type::PINTEREST);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(TumblrType)];
            if (username) {
                card.setHasTumblr(true);
                nets[idx].setType(Network::Type::TUMBLR);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(LinkedInType)];
            if (username) {
                card.setHasLinkedIn(true);
                nets[idx].setType(Network::Type::LINKED_IN);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(PeriscopeType)];
            if (username) {
                card.setHasPeriscope(true);
                nets[idx].setType(Network::Type::PERISCOPE);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(VineType)];
            if (username) {
                card.setHasVine(true);
                nets[idx].setType(Network::Type::VINE);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(SoundCloudType)];
            if (username) {
                card.setHasSoundCloud(true);
                nets[idx].setType(Network::Type::SOUND_CLOUND);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(SinaWeiboType)];
            if (username) {
                card.setHasSinaWeibo(true);
                nets[idx].setType(Network::Type::SINA_WEIBO);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(VKontakteType)];
            if (username) {
                card.setHasVKontakte(true);
                nets[idx].setType(Network::Type::V_KONTAKTE);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }
        }
    }

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    return [NSData dataWithBytes: from length: size];
}

// TODO(Erich): This shares a lot of code with newCreateCardRequest...
// The main difference is including the id and version properties, and
// making an "update card" request.
+ (NSData *)
newUpdateCardRequestWithId: (NSInteger)      requestId
properties:                 (NSDictionary *) props
{
    capnp::MallocMessageBuilder builder;
    Request::Builder request = builder.initRoot<Request>();
    request.setId(requestId);

    UpdateCardRequest::Builder cardRequest = request.getKind().initUpdateCardRequest();
    Card::Builder card = cardRequest.initCard();

    if (props) {
        NSNumber *cardId = props[@"id"];
        assert(cardId);
        card.setId([cardId integerValue]);

        NSNumber *version = props[@"version"];
        assert(version);
        card.setVersion([version intValue]);

        NSString *fullName = props[@"fullName"];
        if (fullName) card.setFullName([fullName UTF8String]);

        NSString *location = props[@"location"];
        if (location) card.setLocation([location UTF8String]);

        NSString *bio = props[@"bio"];
        if (bio) card.setBio([bio UTF8String]);

        NSString *avatarURLString = props[@"avatarURLString"];
        if (avatarURLString) card.setAvatarURLString([avatarURLString UTF8String]);

        NSString *backgroundURLString = props[@"backgroundURLString"];
        if (backgroundURLString) card.setBackgroundURLString([backgroundURLString UTF8String]);

        NSDictionary *networks = props[@"networks"];
        if (networks) {
            NSInteger count = [[networks allKeys] count];

            auto nets = card.initNetworks((unsigned int)count);
            unsigned int idx = 0;

            NSString *username = networks[@(FacebookType)];
            if (username) {
                card.setHasFacebook(true);
                nets[idx].setType(Network::Type::FACEBOOK);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(TwitterType)];
            if (username) {
                card.setHasTwitter(true);
                nets[idx].setType(Network::Type::TWITTER);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(InstagramType)];
            if (username) {
                card.setHasInstagram(true);
                nets[idx].setType(Network::Type::INSTAGRAM);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(SnapchatType)];
            if (username) {
                card.setHasSnapchat(true);
                nets[idx].setType(Network::Type::SNAPCHAT);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(GooglePlusType)];
            if (username) {
                card.setHasGooglePlus(true);
                nets[idx].setType(Network::Type::GOOGLE_PLUS);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(YouTubeType)];
            if (username) {
                card.setHasYouTube(true);
                nets[idx].setType(Network::Type::YOU_TUBE);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(PinterestType)];
            if (username) {
                card.setHasPinterest(true);
                nets[idx].setType(Network::Type::PINTEREST);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(TumblrType)];
            if (username) {
                card.setHasTumblr(true);
                nets[idx].setType(Network::Type::TUMBLR);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(LinkedInType)];
            if (username) {
                card.setHasLinkedIn(true);
                nets[idx].setType(Network::Type::LINKED_IN);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(PeriscopeType)];
            if (username) {
                card.setHasPeriscope(true);
                nets[idx].setType(Network::Type::PERISCOPE);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(VineType)];
            if (username) {
                card.setHasVine(true);
                nets[idx].setType(Network::Type::VINE);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(SoundCloudType)];
            if (username) {
                card.setHasSoundCloud(true);
                nets[idx].setType(Network::Type::SOUND_CLOUND);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(SinaWeiboType)];
            if (username) {
                card.setHasSinaWeibo(true);
                nets[idx].setType(Network::Type::SINA_WEIBO);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }

            username = networks[@(VKontakteType)];
            if (username) {
                card.setHasVKontakte(true);
                nets[idx].setType(Network::Type::V_KONTAKTE);
                nets[idx].setUsername([username UTF8String]);
                idx++;
            }
        }
    }

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());
    
    return [NSData dataWithBytes: from length: size];
}

+ (NSData *)
newCardRequestWithId: (NSInteger) requestId
cardId:               (NSInteger) cardId
version:              (int)       version;
{
    capnp::MallocMessageBuilder builder;

    Request::Builder request = builder.initRoot<Request>();
    request.setId(requestId);

    auto cardRequest = request.getKind().initCardRequest();
    cardRequest.setId(cardId);
    cardRequest.setVersion(version < 0 ? 0 : version);

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    return [NSData dataWithBytes: from length: size];
}

+ (NSData *)
newCreateBackupRequestWithId: (NSInteger) requestId
timestamp:                    (int64_t)   timestamp
previous:                     (int64_t)   previous
{
    capnp::MallocMessageBuilder builder;

    Request::Builder request = builder.initRoot<Request>();
    request.setId(requestId);

    auto createBackupRequest = request.getKind().initCreateBackupRequest();
    createBackupRequest.setPreviousBackup(previous);

    auto backup = createBackupRequest.initBackup();
    backup.setTimestamp(timestamp);

    // We need to create the actual backup, turn it into a buffer, and set it as the "data" property of the backup.
    NSData *backupData = [self clientBackupWithTimestamp: timestamp];
    if (!backupData) return nil;

    const char *backupBytes = (const char *)[backupData bytes];
    unsigned int backupSize = (unsigned int)[backupData length];

    // TODO(Erich): Is there a faster way to do this?
    auto data = backup.initData(backupSize);
    for (size_t idx=0, len=backupSize; idx<len; ++idx) {
        data[idx] = backupBytes[idx];
    }

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    return [NSData dataWithBytes: from length: size];
}

+ (NSData *) newBackupRequestWithId: (NSInteger) requestId
{
    capnp::MallocMessageBuilder builder;

    Request::Builder request = builder.initRoot<Request>();
    request.setId(requestId);

    auto backupRequest = request.getKind().initBackupRequest();
    backupRequest.setTimestamp(0);

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    return [NSData dataWithBytes: from length: size];
}

+ (NSData *) clientBackupWithTimestamp: (int64_t) timestamp
{
    capnp::MallocMessageBuilder builder;

    ClientBackup::Builder clientBackup = builder.initRoot<ClientBackup>();
    clientBackup.setTimestamp(timestamp);

    DeviceObject *device = [APP_DELEGATE.blueModel activeDevice];
    NSAssert(!device.needsToBeCreated, @"Active device should already be created!");

    NSArray *visibleCards = device.visibleCards;

    auto backupCardList = clientBackup.getKind().initBackupCardList((unsigned int)[visibleCards count]);
    for (unsigned int idx=0, len=backupCardList.size(); idx<len; ++idx) {
        CardObject *card = [CardObject objectForPrimaryKey: visibleCards[idx]];

        if (card) {
            auto backupCard = backupCardList[idx];
            backupCard.setId(card.id);
            backupCard.setTimestamp([card.timestamp millisecondsSince1970]);

            backupCard.setIsBLECard(card.isBLECard);
            backupCard.setIsBlueCardLink(card.isFromBlueCardLink);

            backupCard.setFullName([card.fullName UTF8String]);

            NSString *location = card.location;
            if (location && location.length > 0) {
                backupCard.setLocation([location UTF8String]);
            }
        }
    }

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    NSData *uncompressed = [NSData dataWithBytes: from length: size];

    NSError *error = nil;
    NSData *compressed = [BZipCompression compressedDataWithData: uncompressed
                                          blockSize:              BZipDefaultBlockSize
                                          workFactor:             BZipDefaultWorkFactor
                                          error:                  &error               ];

    if (error) {
        NSLog(@"BZipCompression error: %@", error);
    }

    return compressed;
}

@end
