//
//  MediaViewController.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import <LayerKit/LayerKit.h>

@interface MediaViewController : UIViewController

/**
 @abstact Initializes the controller with a message object.
 @discussion The message object should contain message parts with MIMETypes of `ATLMIMETypeImageJPEG`, `ATLMIMETypeImageJPEGPreview`, `ATLMIMETypeImageSize`.
 */
- (instancetype) initWithMessage: (LYRMessage *) message;

@end
