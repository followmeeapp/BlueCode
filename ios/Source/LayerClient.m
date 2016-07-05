//
//  LayerClient.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "LayerClient.h"

@implementation LayerClient

- (NSUInteger) countOfUnreadMessages
{
    LYRQuery *query = [LYRQuery queryWithQueryableClass: LYRMessage.class];

    LYRPredicate *unreadPred =[LYRPredicate predicateWithProperty: @"isUnread"
                                            predicateOperator:     LYRPredicateOperatorIsEqualTo
                                            value:                 @(YES)                       ];

    LYRPredicate *userPred = [LYRPredicate predicateWithProperty: @"sender.userID"
                                           predicateOperator:     LYRPredicateOperatorIsNotEqualTo
                                           value:                 self.authenticatedUser.userID   ];

    query.predicate = [LYRCompoundPredicate compoundPredicateWithType: LYRCompoundPredicateTypeAnd
                                            subpredicates:             @[unreadPred, userPred]    ];

    return [self countForQuery: query error: nil];
}

- (NSUInteger) countOfMessages
{
    LYRQuery *query = [LYRQuery queryWithQueryableClass: LYRMessage.class];

    return [self countForQuery: query error: nil];
}

- (NSUInteger) countOfConversations
{
    LYRQuery *query = [LYRQuery queryWithQueryableClass: LYRConversation.class];

    return [self countForQuery: query error: nil];
}

- (LYRMessage *) messageForIdentifier: (NSURL *) identifier
{
    LYRQuery *query = [LYRQuery queryWithQueryableClass: LYRMessage.class];

    query.predicate = [LYRPredicate predicateWithProperty: @"identifier"
                                    predicateOperator:     LYRPredicateOperatorIsEqualTo
                                    value:                 identifier                   ];
    query.limit = 1;

    return [self executeQuery: query error: nil].firstObject;
}

- (LYRConversation *) existingConversationForIdentifier: (NSURL *) identifier
{
    LYRQuery *query = [LYRQuery queryWithQueryableClass: LYRConversation.class];

    query.predicate = [LYRPredicate predicateWithProperty: @"identifier"
                                    predicateOperator:     LYRPredicateOperatorIsEqualTo
                                    value:                 identifier                   ];
    query.limit = 1;

    return [self executeQuery: query error: nil].firstObject;
}

- (LYRConversation *) existingConversationForParticipants: (NSSet *) participants
{
    LYRQuery *query = [LYRQuery queryWithQueryableClass: LYRConversation.class];

    query.predicate = [LYRPredicate predicateWithProperty: @"participants"
                                    predicateOperator:     LYRPredicateOperatorIsEqualTo
                                    value:                 participants                 ];
    query.limit = 1;

    return [self executeQuery: query error: nil].firstObject;
}

@end
