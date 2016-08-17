//
//  BlueDeepLink.swift
//  Follow
//
//  Created by Erich Ocean on 7/31/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

import UIKit
import Appz

@objc public class BlueDeepLink : NSObject {

    @objc public func openTwitter(userHandle: String) {
        let app = UIApplication.sharedApplication()
        app.open(Applications.Twitter(), action: .UserHandle(userHandle))
    }

    @objc public func openInstagram(username: String) {
        let app = UIApplication.sharedApplication()
        app.open(Applications.Instagram(), action: .Username(username: username))
    }

    @objc public func openSnapchat(username: String) {
        let app = UIApplication.sharedApplication()
        app.open(Applications.Snapchat(), action: .Add(username: username))
    }

    @objc public func openPinterest(name: String) {
        let app = UIApplication.sharedApplication()
        app.open(Applications.Pinterest(), action: .User(name: name))
    }

}
