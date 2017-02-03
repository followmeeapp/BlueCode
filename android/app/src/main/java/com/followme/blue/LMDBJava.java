package com.followme.blue;

/**
 * Created by erik on 02/02/2017.
 */

class LMDBJava {
    static {
        System.loadLibrary("lmdb");
    }
    private long env_;

    public void open(String envPath) {
        env_ = openEnv(envPath);
    }

    public void close() {
        closeEnv(env_);
    }

    public long openDB() {
        return openDBI(env_);
    }

    public void closeDB(long dbi) {
        closeDBI(env_, dbi);
    }

    public int put(long dbi, int key, String value) {
        return put(env_, dbi, key, value);
    }

    public String get(long dbi, int key) {
        return get(env_, dbi, key);
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    private native long openEnv(String dir);
    private native void closeEnv(long env);
    private native long openDBI(long env);
    private native void closeDBI(long env, long dbi);
    private native int put(long env, long dbi, int key, String value);
    private native String get(long env, long dbi, int key);
}
