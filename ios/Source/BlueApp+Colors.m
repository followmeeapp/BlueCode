//
//  BlueApp+Colors.m
//  Follow
//
//  Created by Erich Ocean on 7/30/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueApp.h"

@interface UIColor (Hexadecimal)

+ (UIColor *) colorWithHexString: (NSString *) hexString;

@end

@implementation BlueApp (Colors)

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

- (UIColor *) navBarColor
{
    static dispatch_once_t pred;
    static UIColor *color = nil;

    dispatch_once(&pred, ^{
        color = [UIColor colorWithHexString: @"#330C00"];
    });

    return color;
}

- (UIColor *) bluetoothBlue
{
    static dispatch_once_t pred;
    static UIColor *color = nil;

    dispatch_once(&pred, ^{
        color = [UIColor colorWithHexString: @"#0082FC"]; // rgb(0,130,252)
    });

    return color;
}

- (UIColor *) instinctColor
{
    static dispatch_once_t pred;
    static UIColor *color = nil;

    dispatch_once(&pred, ^{
        color = [UIColor colorWithRed: (255.0/255.0) green: (211.0/255.0) blue: (3.0/255.0) alpha: 1.0];
    });

    return color;
}

- (UIColor *) mysticColor
{
    static dispatch_once_t pred;
    static UIColor *color = nil;

    dispatch_once(&pred, ^{
        color = [UIColor colorWithRed: (2.0/255.0) green: (118.0/255.0) blue: (242.0/255.0) alpha: 1.0];
    });

    return color;
}

- (UIColor *) valorColor
{
    static dispatch_once_t pred;
    static UIColor *color = nil;

    dispatch_once(&pred, ^{
        color = [UIColor colorWithRed: (247.0/255.0) green: (17.0/255.0) blue: (7.0/255.0) alpha: 1.0];
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
