To get an Xcode project, do:

    bazel build //b:objc-main --ios_sdk_version=9.3

You'll something like this printed:

    Target //b:objc-main up-to-date:
      bazel-bin/b/objc-main_bin
      bazel-bin/b/objc-main.ipa
      bazel-bin/b/objc-main.xcodeproj/project.pbxproj

Based on that, do:

     open bazel-bin/b/objc-main.xcodeproj

That will open the Xcode workspace. You can now run the project in the iOS simulator and set breakpoints in individual files.