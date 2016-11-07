//
//  SectionListBuilder.h
//  Follow
//
//  Created by Erich Ocean on 10/13/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

@interface SectionListBuilder : NSObject

+ (NSData *) dataWithSections: (NSArray *) sections;

+ (NSArray *) sectionsFromData: (NSData *) data;

@end
