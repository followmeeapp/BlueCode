//
//  InputTableViewCell.h
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

@interface InputTableViewCell : UITableViewCell

@property (nonatomic) UITextField *textField;

- (void) setGuideText: (NSString *) guideText;

- (void) setPlaceHolderText: (NSString *) placeHolderText;

@end
