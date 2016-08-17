//
//  BluePokemonGoController.m
//  Follow
//
//  Created by Erich Ocean on 8/15/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BluePokemonGoController.h"

#import "BlueApp.h"

#import "NetworkObject.h"

@implementation BluePokemonGoController

- (void) viewDidLoad
{
    [super viewDidLoad];

    self.avatar.layer.cornerRadius = 78.0;

    [self updateAvatar];
}

- (void) updateAvatar
{
    NSString *team = APP_DELEGATE.networks[@(PokemonGoType)];

    CALayer *layer = self.avatar.layer;

    if (!team) {
        layer.borderColor = [UIColor blackColor].CGColor;
        layer.borderWidth = 1.0;

    } else if ([team isEqualToString: @"instinct"]) {
        layer.borderColor = [APP_DELEGATE instinctColor].CGColor;
        layer.borderWidth = 12.0;

    } else if ([team isEqualToString: @"mystic"]) {
        layer.borderColor = [APP_DELEGATE mysticColor].CGColor;
        layer.borderWidth = 12.0;

    } else if ([team isEqualToString: @"valor"]) {
        layer.borderColor = [APP_DELEGATE valorColor].CGColor;
        layer.borderWidth = 12.0;
    }
}

- (IBAction) chooseTeamColor: sender
{
    switch ([sender tag]) {
        case 0: {
            [APP_DELEGATE.networks removeObjectForKey: @(PokemonGoType)];
        } break;

        case 1: {
            APP_DELEGATE.networks[@(PokemonGoType)] = @"instinct";
        } break;

        case 2: {
            APP_DELEGATE.networks[@(PokemonGoType)] = @"mystic";
        } break;

        case 3: {
            APP_DELEGATE.networks[@(PokemonGoType)] = @"valor";
        } break;
    }

    [self updateAvatar];
}

@end
