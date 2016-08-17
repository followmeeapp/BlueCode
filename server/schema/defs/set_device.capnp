#
#  set_device.capnp
#  Follow
#
# Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
#

@0xae95d9cbcb398a3d;

struct SetDeviceRequest {
  telephone @0 :Text;
}

struct SetDeviceResponse {
  deviceId @0 :Int64;
}
