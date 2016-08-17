//
//  BlueModel.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueModel.h"

#import <Realm/Realm.h>

#import "UserObject.h"
#import "CardObject.h"

@implementation BlueModel

- (instancetype) init
{
    if (self = [super init]) {
//        [self configureRealm: nil];
    }

    return self;
}

- (void) configureRealm: (UIApplication *) application
{
    // Get the default Realm. You only need to do this once (per thread).
    RLMRealm *realm = [RLMRealm defaultRealm];

    // Destroy existing objects.
    [realm beginWriteTransaction];
    [realm deleteAllObjects];
    [realm commitWriteTransaction];

    //    // Create hard-coded User object.
    //    UserObject *user = [[UserObject alloc] init];
    //    user.telephone = @"3234823204";
    //
    //    // Add User object to Realm with transaction.
    //    [realm beginWriteTransaction];
    //    [realm addObject: user];
    //    [realm commitWriteTransaction];
}

- (UserObject *) activeUser
{
    return [[UserObject allObjects] firstObject];
}

- (CardObject *) activeUserCard
{
    UserObject *user = [self activeUser];
    if (!user) return nil;

    NSInteger cardId = user.cardId;
    if (cardId == 0) return nil;

//    NSPredicate *pred = [NSPredicate predicateWithFormat: @"id = %@", @(cardId)];
//    return [[CardObject objectsWithPredicate: pred] firstObject];
    return [CardObject objectForPrimaryKey: @(cardId)];
}

@end
