//
//  BlueIntroScreenController.h
//  Follow
//
//  Created by Erich Ocean on 10/5/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueIntroController.h"

@interface BlueIntroScreenController : UIViewController

@property (nonatomic, assign) IntroStatus status;
@property (nonatomic, weak) BlueIntroController *introController;

- (instancetype)
initWithVideo: (NSString *)  videoName
status:        (IntroStatus) status;

- (void) reset;
- (void) play;
- (void) pause;

@end
