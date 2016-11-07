//
//  CardObject.m
//  Follow
//
//  Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#import "CardObject.h"

#import "SectionListBuilder.h"

@interface CardObject ()

@property (nonatomic, strong) NSArray *internalSections;

@end

@implementation CardObject

+ (NSString *) primaryKey
{
    return @"id";
}

+ (NSArray *) indexedProperties
{
    return @[@"version", @"isFromBackup"];
}

+ (NSDictionary *) defaultPropertyValues
{
    return @{
        @"version": @0, // Implies that the Card has never been saved to the server.
        @"timestamp": [NSDate dateWithTimeIntervalSinceNow: 0],
        @"isBLECard": @NO,
        @"isFromBlueCardLink": @NO,
        @"isFromBackup": @NO,
        @"status": @"",
        @"statusUpdatedAt": [NSDate distantPast],
        @"visibleSection": @0,
        @"backgroundOpacity": @(0.5),
        @"rowOffset": @(0),
        @"hasFacebook": @NO,
        @"hasTwitter": @NO,
        @"hasInstagram": @NO,
        @"hasSnapchat": @NO,
        @"hasGooglePlus": @NO,
        @"hasYouTube": @NO,
        @"hasPinterest": @NO,
        @"hasTumblr": @NO,
        @"hasLinkedIn": @NO,
        @"hasPeriscope": @NO,
        @"hasVine": @NO,
        @"hasSoundCloud": @NO,
        @"hasSinaWeibo": @NO,
        @"hasVKontakte": @NO,

        // Card analytics
        @"cardBLEDeliveries": @(0),
        @"cardLinkDeliveries": @(0),
        @"cardViews": @(0),
        @"cardShares": @(0),
        @"facebookViews": @(0),
        @"twitterViews": @(0),
        @"instagramViews": @(0),
        @"snapchatViews": @(0),
        @"googlePlusViews": @(0),
        @"youTubeViews": @(0),
        @"pinterestViews": @(0),
        @"tumblrViews": @(0),
        @"linkedInViews": @(0),
        @"periscopeViews": @(0),
        @"vineViews": @(0),
        @"soundCloudViews": @(0),
        @"sinaWeiboViews": @(0),
        @"vKontakteViews": @(0),
    };
}

+ (NSArray *) ignoredProperties
{
    return @[@"internalSections"];
}

- (NSArray *) sections
{
    NSArray *sections = self.internalSections;

    if (!sections) [self loadSections];

    return self.internalSections;
}

- (void) updateSections: (NSArray *) sections;
{
    self.sectionData = [SectionListBuilder dataWithSections: sections];

    // Update internal cache.
    self.internalSections = [sections copy];
}

- (void) loadSections
{
    NSData *data = self.sectionData;

    self.internalSections = data ? [SectionListBuilder sectionsFromData: data] : @[];
}

@end
