#
#  test_processes.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0x9becb3f0024216cc;

struct TestProcesses {
  servers @0 : List(Server);
}

struct Server {
  settings : union {
    server                @0 : import "settings.capnp".ServerSettings;
    discoveryd            @1 : import "settings.capnp".DiscoverydSettings;
    eventLogger           @2 : import "settings.capnp".EventLoggerSettings;
    requestHistogrammer   @3 : import "settings.capnp".RequestHistogrammerSettings;
  }
  clockRegion @4: Text;
}

