package com.followme.blue;

/**
 * Created by erik on 02/02/2017.
 */

public class CapnpEcho {
    static {
        System.loadLibrary("capnp_echo");
    }

    // this example is so simple I just directly expose the native method
    public native byte[] echoCard(byte[] buffer);
}
