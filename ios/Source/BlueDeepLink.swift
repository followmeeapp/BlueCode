//
//  BlueDeepLink.swift
//  Follow
//
//  Created by Erich Ocean on 7/31/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

import UIKit
import Appz

@objc open class BlueDeepLink : NSObject {

    @objc open func openTwitter(_ userHandle: String) {
        let app = UIApplication.shared
        app.open(Applications.Twitter(), action: .userHandle(userHandle))
    }

    @objc open func openInstagram(_ username: String) {
        let app = UIApplication.shared
        app.open(Applications.Instagram(), action: .username(username: username))
    }

    @objc open func openSnapchat(_ username: String) {
        let app = UIApplication.shared
        app.open(Applications.Snapchat(), action: .add(username: username))
    }

    @objc open func openPinterest(_ name: String) {
        let app = UIApplication.shared
        app.open(Applications.Pinterest(), action: .user(name: name))
    }

}
