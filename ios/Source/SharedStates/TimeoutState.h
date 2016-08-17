//
//  TimeoutState.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "EOState.h"

@interface TimeoutState : EOState

@property (nonatomic, assign) double timeout;

- (void) showAlert: (NSString *) title;
- (void) showNotImplementedAlert: (SEL) selector;

@end
