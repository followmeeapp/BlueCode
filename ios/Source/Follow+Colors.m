//
//  Follow+Colors.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "Follow.h"

@interface UIColor (Hexadecimal)

+ (UIColor *) colorWithHexString: (NSString *) hexString;

@end

@implementation Follow (Colors)

- (UIColor *) blue1
{
    static dispatch_once_t pred;
    static UIColor *color = nil;

    dispatch_once(&pred, ^{
        color = [UIColor colorWithHexString: @"#0069a7"];
    });

    return color;
}

- (UIColor *) blue2
{
    static dispatch_once_t pred;
    static UIColor *color = nil;

    dispatch_once(&pred, ^{
        color = [UIColor colorWithHexString: @"#008cc0"];
    });

    return color;
}

- (UIColor *) blue3
{
    static dispatch_once_t pred;
    static UIColor *color = nil;

    dispatch_once(&pred, ^{
        color = [UIColor colorWithHexString: @"#00b5d9"];
    });

    return color;
}

- (UIColor *) blue4
{
    static dispatch_once_t pred;
    static UIColor *color = nil;

    dispatch_once(&pred, ^{
        color = [UIColor colorWithHexString: @"#00f1f1"];
    });

    return color;
}

- (UIColor *) blue5
{
    static dispatch_once_t pred;
    static UIColor *color = nil;

    dispatch_once(&pred, ^{
        color = [UIColor colorWithHexString: @"#0069a7"];
    });

    return color;
}

- (UIColor *) black
{
    static dispatch_once_t pred;
    static UIColor *color = nil;

    dispatch_once(&pred, ^{
        color = [UIColor colorWithHexString: @"#2b2b2b"];
    });
    
    return color;
}

@end

// From: http://stackoverflow.com/a/27042677
@implementation UIColor (Hexadecimal)

+ (UIColor *) colorWithHexString: (NSString *) hexString
{
    unsigned rgbValue = 0;
    NSScanner *scanner = [NSScanner scannerWithString: hexString];
    [scanner setScanLocation: 1]; // bypass '#' character
    [scanner scanHexInt: &rgbValue];

    return [UIColor colorWithRed: ((rgbValue & 0xFF0000) >> 16)/255.0 green: ((rgbValue & 0xFF00) >> 8)/255.0 blue: (rgbValue & 0xFF)/255.0 alpha: 1.0];
}

@end
