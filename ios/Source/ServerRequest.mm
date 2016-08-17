//
//  ServerRequest.mm
//  Follow
//
//  Created by Erich Ocean on 7/13/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "ServerRequest.h"

#import "NetworkObject.h"

#include <iostream>
#include <string>

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/request.capnp.h"
#include "schema/card.capnp.h"

@implementation ServerRequest

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

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    return [NSData dataWithBytes: from length: size];
}

+ (NSData *)
newJoinRequestWithId: (NSInteger) requestId
telephone:            (NSString *) telephone;
{
    capnp::MallocMessageBuilder builder;
    Request::Builder request = builder.initRoot<Request>();
    request.setId(requestId);

    NSUUID *UUID = [[UIDevice currentDevice] identifierForVendor];

    JoinRequest::Builder joinRequest = request.getKind().initJoinRequest();
    joinRequest.setTelephone([telephone UTF8String]);

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

            username = networks[@(PokemonGoType)];
            if (username) {
                card.setHasPokemonGo(true);
                nets[idx].setType(Network::Type::POKEMON_GO);
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

            username = networks[@(PokemonGoType)];
            if (username) {
                card.setHasPokemonGo(true);
                nets[idx].setType(Network::Type::POKEMON_GO);
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
    cardRequest.setVersion(version);

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *from = (char *)(bytes.begin());

    return [NSData dataWithBytes: from length: size];
}

@end
