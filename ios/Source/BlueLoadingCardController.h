//
//  BlueLoadingCardController.h
//  Follow
//
//  Created by Erich Ocean on 10/23/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

@class CardObject;

@interface BlueLoadingCardController : UIViewController

@property (nonatomic, strong) CardObject *card;

@property (nonatomic, weak) IBOutlet UILabel *fullName;
@property (nonatomic, weak) IBOutlet UILabel *location;

@end
