//
//  BlueWebController.m
//  Follow
//
//  Created by Erich Ocean on 11/3/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueWebController.h"

#import <WebKit/WebKit.h>

@interface BlueWebController () <WKNavigationDelegate>

@end

@implementation BlueWebController

- (void) viewDidLoad
{
    [super viewDidLoad];

    WKWebViewConfiguration *config = [[WKWebViewConfiguration alloc] init];
    WKWebView *webView = [[WKWebView alloc] initWithFrame: self.view.frame configuration: config];

    webView.navigationDelegate = self;

    NSURLRequest *request = [NSURLRequest requestWithURL: self.url];
    [webView loadRequest: request];

    [self.view addSubview: webView];
}

@end
