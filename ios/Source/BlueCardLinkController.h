//
//  BlueCardLinkController.h
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

@interface BlueCardLinkController : UINavigationController

@property (nonatomic, assign) NSInteger cardId;
@property (nonatomic, copy) NSString *fullName;
@property (nonatomic, copy) NSString *location;

- (void) configure;

@end
