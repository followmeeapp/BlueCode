//
//  ServerResponse.mm
//  Follow
//
//  Created by Erich Ocean on 7/13/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "ServerResponse.h"

#import "BlueApp.h"

#import <Realm/Realm.h>

#import "BlueModel.h"

#import "UserObject.h"
#import "DeviceObject.h"
#import "CardObject.h"
#import "NetworkObject.h"
#import "SectionObject.h"

#include <iostream>
#include <string>

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/response.capnp.h"

@implementation ServerResponse

+ (void) handleResponse: (NSData *) data
{
    kj::ArrayPtr<const capnp::word> view((const capnp::word *)[data bytes], [data length] / 8);
    capnp::FlatArrayMessageReader reader(view);

    Response::Reader response = reader.getRoot<Response>();
    auto requestId = response.getId();
    auto kind = response.getKind();
    auto which = kind.which();

    RLMRealm *realm = [RLMRealm defaultRealm];

    if (which == Response::Kind::ERROR_RESPONSE) {
        auto errorResponse = kind.getErrorResponse();
        NSLog(@"ERROR: %s", errorResponse.getMessage().cStr());

    } else if (which == Response::Kind::HELLO_RESPONSE) {
        auto helloResponse = kind.getHelloResponse();
        NSLog(@"HELLO: %@", @(helloResponse.hasDiscoveryResponse()));

    } else if (which == Response::Kind::JOIN_RESPONSE) {
      auto joinResponse = kind.getJoinResponse();
      NSLog(@"JOIN");

      if (joinResponse.getStatus() == JoinResponse::Status::NEW) {
        [[NSNotificationCenter defaultCenter] postNotificationName: @"NewUser" object: nil];

        [realm beginWriteTransaction];
        auto user = joinResponse.getUser();
        [self insertUser: user inRealm: realm];
        [realm commitWriteTransaction];

      } else {
        [[NSNotificationCenter defaultCenter] postNotificationName: @"ExistingUser" object: nil];

        [realm beginWriteTransaction];
        auto user = joinResponse.getUser();
        [self insertUser: user inRealm: realm];

        if (joinResponse.hasCard()) {
          auto card = joinResponse.getCard();
          [self insertOwnCard: card inRealm: realm];
        }

        [realm commitWriteTransaction];
      }

      [[NSNotificationCenter defaultCenter] postNotificationName: @"DidCreateUser" object: nil];
      if (joinResponse.hasCard()) {
        [[NSNotificationCenter defaultCenter] postNotificationName: @"DidCreateCard" object: nil];
      }

    } else if (which == Response::Kind::CARD_RESPONSE) {
        auto cardResponse = kind.getCardResponse();
        NSLog(@"CARD: %@", @(cardResponse.getCard().getId()));

        auto status = cardResponse.getStatus();
        if (status == CardResponse::Status::CREATED) {
            [realm beginWriteTransaction];
            auto card = cardResponse.getCard();
            [self insertOwnCard: card inRealm: realm];
            [realm commitWriteTransaction];
            [[NSNotificationCenter defaultCenter] postNotificationName: @"DidCreateCard" object: nil];

        } else if (status == CardResponse::Status::UPDATED) {
            [realm beginWriteTransaction];
            auto card = cardResponse.getCard();
            [self insertCard: card inRealm: realm];
            [realm commitWriteTransaction];
            [[NSNotificationCenter defaultCenter] postNotificationName: @"DidUpdateCard" object: @(card.getId())];

        } else {
            // Card is current; nothing to do.
        }

//    } else if (which == Response::Kind::YOUR_CARD) {
//        auto myCard = kind.getYourCard();
//        NSLog(@"YOUR CARD: %@", @(myCard.getId()));
//
//        [realm beginWriteTransaction];
//        [self insertOwnCard: myCard inRealm: realm];
//        [realm commitWriteTransaction];

    } else {
        NSLog(@"Uknown response.");
    }
}

+ (void)
insertCard: (Card::Reader &) card
inRealm:    (RLMRealm *)     realm
{
    NSInteger cardId = card.getId();
    int version = card.getVersion();

//    NSPredicate *pred = [NSPredicate predicateWithFormat: @"id = %@", @(cardId)];
//    CardObject *existingCard = [[CardObject objectsWithPredicate: pred] firstObject];
    CardObject *existingCard = [CardObject objectForPrimaryKey: @(cardId)];

    if (existingCard && existingCard.version == version) return;
        // Nothing to do.

    if (!existingCard) {
        existingCard = [[CardObject alloc] init];
        existingCard.id = cardId;
    }

    existingCard.version = version;
    existingCard.fullName  = card.hasFullName() ? [NSString stringWithUTF8String: card.getFullName().cStr()] : nil;
    existingCard.location  = card.hasLocation() ? [NSString stringWithUTF8String: card.getLocation().cStr()] : nil;
    existingCard.bio       = card.hasBio()      ? [NSString stringWithUTF8String: card.getBio().cStr()]      : nil;

    existingCard.avatarURLString     = card.hasAvatarURLString()     ? [NSString stringWithUTF8String: card.getAvatarURLString().cStr()]     : nil;
    existingCard.backgroundURLString = card.hasBackgroundURLString() ? [NSString stringWithUTF8String: card.getBackgroundURLString().cStr()] : nil;

    // Facebook
    if (card.getHasFacebook()) {
        if (existingCard.hasFacebook) {
            // Make sure the existing Facebook network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(FacebookType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::FACEBOOK) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasFacebook = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = FacebookType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::FACEBOOK) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasFacebook) {
            // Remove the existing Facebook network object.
            existingCard.hasFacebook = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(FacebookType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // Twitter
    if (card.getHasTwitter()) {
        if (existingCard.hasTwitter) {
            // Make sure the existing Twitter network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(TwitterType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::TWITTER) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasTwitter = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = TwitterType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::TWITTER) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasTwitter) {
            // Remove the existing Twitter network object.
            existingCard.hasTwitter = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(TwitterType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // Instagram
    if (card.getHasInstagram()) {
        if (existingCard.hasInstagram) {
            // Make sure the existing Instagram network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(InstagramType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::INSTAGRAM) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasInstagram = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = InstagramType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::INSTAGRAM) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasInstagram) {
            // Remove the existing Instagram network object.
            existingCard.hasInstagram = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(InstagramType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // Pokémon Go
    if (card.getHasPokemonGo()) {
        if (existingCard.hasPokemonGo) {
            // Make sure the existing Pokémon Go network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(PokemonGoType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::POKEMON_GO) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasPokemonGo = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = PokemonGoType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::POKEMON_GO) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasPokemonGo) {
            // Remove the existing Pokémon Go network object.
            existingCard.hasPokemonGo = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(PokemonGoType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // Snapchat
    if (card.getHasSnapchat()) {
        if (existingCard.hasSnapchat) {
            // Make sure the existing Snapchat network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(SnapchatType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::SNAPCHAT) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasSnapchat = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = SnapchatType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::SNAPCHAT) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasSnapchat) {
            // Remove the existing Snapchat network object.
            existingCard.hasSnapchat = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(SnapchatType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // Google+
    if (card.getHasGooglePlus()) {
        if (existingCard.hasGooglePlus) {
            // Make sure the existing Google+ network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(GooglePlusType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::GOOGLE_PLUS) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasGooglePlus = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = GooglePlusType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::GOOGLE_PLUS) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasGooglePlus) {
            // Remove the existing Google+ network object.
            existingCard.hasGooglePlus = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(GooglePlusType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // YouTube
    if (card.getHasYouTube()) {
        if (existingCard.hasYouTube) {
            // Make sure the existing YouTube network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(YouTubeType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::YOU_TUBE) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasYouTube = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = YouTubeType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::YOU_TUBE) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasYouTube) {
            // Remove the existing YouTube network object.
            existingCard.hasYouTube = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(YouTubeType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // Pinterest
    if (card.getHasPinterest()) {
        if (existingCard.hasPinterest) {
            // Make sure the existing Pinterest network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(PinterestType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::PINTEREST) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasPinterest = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = PinterestType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::PINTEREST) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasPinterest) {
            // Remove the existing Pinterest network object.
            existingCard.hasPinterest = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(PinterestType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // Tumblr
    if (card.getHasTumblr()) {
        if (existingCard.hasTumblr) {
            // Make sure the existing Tumblr network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(TumblrType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::TUMBLR) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasTumblr = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = TumblrType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::TUMBLR) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasTumblr) {
            // Remove the existing Tumblr network object.
            existingCard.hasTumblr = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(TumblrType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // LinkedIn
    if (card.getHasLinkedIn()) {
        if (existingCard.hasLinkedIn) {
            // Make sure the existing LinkedIn network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(LinkedInType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::LINKED_IN) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasLinkedIn = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = LinkedInType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::LINKED_IN) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasLinkedIn) {
            // Remove the existing LinkedIn network object.
            existingCard.hasLinkedIn = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(LinkedInType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // Periscope
    if (card.getHasPeriscope()) {
        if (existingCard.hasPeriscope) {
            // Make sure the existing Periscope network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(PeriscopeType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::PERISCOPE) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasPeriscope = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = PeriscopeType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::PERISCOPE) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasPeriscope) {
            // Remove the existing Periscope network object.
            existingCard.hasPeriscope = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(PeriscopeType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // Vine
    if (card.getHasVine()) {
        if (existingCard.hasVine) {
            // Make sure the existing Vine network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(VineType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::VINE) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasVine = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = VineType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::VINE) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasVine) {
            // Remove the existing Vine network object.
            existingCard.hasVine = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(VineType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // SoundCloud
    if (card.getHasSoundCloud()) {
        if (existingCard.hasSoundCloud) {
            // Make sure the existing SoundCloud network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(SoundCloudType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::SOUND_CLOUND) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasSoundCloud = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = SoundCloudType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::SOUND_CLOUND) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasSoundCloud) {
            // Remove the existing SoundCloud network object.
            existingCard.hasSoundCloud = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(SoundCloudType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // Sina Weibo
    if (card.getHasSinaWeibo()) {
        if (existingCard.hasSinaWeibo) {
            // Make sure the existing Sina Weibo network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(SinaWeiboType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::SINA_WEIBO) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasSinaWeibo = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = SinaWeiboType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::SINA_WEIBO) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasSinaWeibo) {
            // Remove the existing Sina Weibo network object.
            existingCard.hasSinaWeibo = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(SinaWeiboType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    // VKontakte
    if (card.getHasVKontakte()) {
        if (existingCard.hasVKontakte) {
            // Make sure the existing VKontakte network object has the correct username.
            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(VKontakteType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                for (auto network : card.getNetworks()) {
                    if (network.getType() == Network::Type::V_KONTAKTE) {
                        existingNetwork.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                    }
                }
                [realm addOrUpdateObject: existingNetwork];
            }

        } else {
            existingCard.hasVKontakte = YES;
            NetworkObject *networkObject = [[NetworkObject alloc] init];
            networkObject.guid = [[NSUUID UUID] UUIDString];
            networkObject.type = VKontakteType;
            networkObject.cardId = cardId;
            for (auto network : card.getNetworks()) {
                if (network.getType() == Network::Type::V_KONTAKTE) {
                    networkObject.username = network.hasUsername() ? [NSString stringWithUTF8String: network.getUsername().cStr()] : nil;
                }
            }
            [existingCard.networks addObject: networkObject];
            [realm addOrUpdateObject: networkObject];
        }

    } else {
        if (existingCard.hasVKontakte) {
            // Remove the existing VKontakte network object.
            existingCard.hasVKontakte = NO;

            NSPredicate *pred = [NSPredicate predicateWithFormat: @"cardId = %@ AND type = %@", @(cardId), @(VKontakteType)];
            NetworkObject *existingNetwork = [[NetworkObject objectsWithPredicate: pred] firstObject];
            if (existingNetwork) {
                [realm deleteObject: existingNetwork];
            }
        }
    }

    [realm addOrUpdateObject: existingCard];
}

+ (void)
insertOwnCard: (Card::Reader &) card
inRealm:       (RLMRealm *)     realm
{
    NSInteger cardId = card.getId();

    if (cardId > 0) [self insertCard: card inRealm: realm];

    // Update User object if needed.
    UserObject *existingUser = [[UserObject allObjects] firstObject];

    if (existingUser && existingUser.cardId == cardId) return;
        // Nothing to do.

    if (existingUser) {
        // Need to update existing user.
        existingUser.cardId = cardId;
        [realm addOrUpdateObject: existingUser];

    } else {
        // Shouldn't happen. o_O
        NSLog(@"ERROR: Got own card but have no User object?");
    }
}

+ (void)
insertUser: (User::Reader &) user
inRealm:    (RLMRealm *)     realm
{
  UserObject *userObject = [[UserObject alloc] init];

  userObject.id = user.getId();
  userObject.activeDeviceId = user.getActiveDevice();

  uuid_t uuid;
  auto data = user.getActiveDeviceUUID();

  for (int idx=0, len=16; idx<len; ++idx) {
    uuid[idx] = data[idx];
  }

  NSData *uuidData = [NSData dataWithBytes: uuid length: 16];
  userObject.activeDeviceUUID = uuidData;

  userObject.cardId = user.getCard();

  [realm addOrUpdateObject: userObject];
}

@end
