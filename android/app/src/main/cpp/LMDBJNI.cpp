//
// Created by Erik van der Tier on 02/02/2017.
//
// super simplified JNI based lmdb class
//
#include <liblmdb/src/lmdb.h>
#include <jni.h>
#include <string>
#include <android/log.h>

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "lmdbJNI::", __VA_ARGS__))

#ifdef __cplusplus
extern "C" {
#endif

// create an lmdb env and return a long, so its 'pointer' can be held on to on the java side
jlong Java_com_followme_blue_LMDBJava_openEnv(
        JNIEnv *env,
        jobject /* this */, jstring dir) {

  MDB_env *lenv;

  std::string dbFile = std::string(env->GetStringUTFChars(dir, nullptr));
  LOGI("Starting");

  int res = mdb_env_create(&lenv);
  LOGI("Created env: %d", res);

  res = mdb_env_set_maxreaders(lenv, 4);
  LOGI("set max readers: %d", res);
  res = mdb_env_set_mapsize(lenv, 10485760);
  LOGI("set mapsize: %d", res);

  LOGI("dbfile: %s", dbFile.c_str());
  res = mdb_env_open(lenv, dbFile.c_str(), MDB_NOSYNC, 0664);
  LOGI("env_open: %d", res);
  return reinterpret_cast<jlong>(lenv);
}

// get rid of the env
void Java_com_followme_blue_LMDBJava_closeEnv(
        JNIEnv *env,
        jobject /* this */, jlong lenv_ptr) {
  MDB_env *lenv = reinterpret_cast<MDB_env *>(lenv_ptr);
  mdb_env_close(lenv);
}

jlong Java_com_followme_blue_LMDBJava_openDBI(
        JNIEnv *env,
        jobject /* this */, jlong lenv_ptr) {
  MDB_env *lenv = reinterpret_cast<MDB_env *>(lenv_ptr);
  MDB_txn *txn;
  MDB_dbi dbi;

  LOGI("openDBI start");
  int res = mdb_txn_begin(lenv, NULL, 0, &txn);
  LOGI("txn_begin: %d", res);

  res = mdb_dbi_open(txn, NULL, 0, &dbi);
  LOGI("dbi_open: %d", res);
  LOGI("openDBI end: %ld", (long)dbi);
  res = mdb_txn_commit(txn);
  return (jlong) dbi;
}

void Java_com_followme_blue_LMDBJava_closeDBI(
        JNIEnv *env,
        jobject /* this */, jlong lenv_ptr, jlong dbi) {
  LOGI("closeDBI start");

  MDB_env *lenv = reinterpret_cast<MDB_env *>(lenv_ptr);

  mdb_dbi_close(lenv, (MDB_dbi) dbi);
  LOGI("closeDBI end");
}

jint Java_com_followme_blue_LMDBJava_put(
        JNIEnv *env, jobject obj, jlong lenv_ptr, jlong dbi, jint key, jstring jvalue) {
  MDB_env *lenv = reinterpret_cast<MDB_env *>(lenv_ptr);
  // MDB_dbi dbi;
  LOGI("put start");

  MDB_val valkey, data;
  MDB_txn *txn;

  int res = mdb_txn_begin(lenv, NULL, 0, &txn);
  LOGI("put txn begin: %d", res);
  if (res != 0) return res;

  std::string value = std::string(env->GetStringUTFChars(jvalue, nullptr));
  LOGI("value: %s of size: %ld", value.c_str(), value.size());
  data.mv_size = value.size();
  data.mv_data = (void *) value.c_str();

  valkey.mv_size = sizeof(key);
  valkey.mv_data = (void *)&key;

  res = mdb_put(txn, (MDB_dbi) dbi, &valkey, &data, 0);
  LOGI("put: %d", res);
  if (res != 0) {
    mdb_txn_abort(txn);
    return res;
  }
  res = mdb_txn_commit(txn);
  LOGI("put end %d", res);

  return res;
}

jstring
Java_com_followme_blue_LMDBJava_get(
        JNIEnv *env, jobject obj, jlong lenv_ptr, jlong dbi, jint key) {
  MDB_env *lenv = reinterpret_cast<MDB_env *>(lenv_ptr);
  LOGI("get start");
  //MDB_dbi dbi;
  MDB_val valkey, result;
  MDB_txn *txn;

  int res = mdb_txn_begin(lenv, NULL, MDB_RDONLY, &txn);
  if (res != 0) return env->NewStringUTF("error");

  valkey.mv_size = sizeof(key);
  valkey.mv_data = (void *) &key;

  res = mdb_get(txn, (MDB_dbi) dbi, &valkey, &result);
  LOGI("get: %d", res);
  if (res != 0) {
    mdb_txn_abort(txn);
    return env->NewStringUTF("error");
  }
  jstring jresult = env->NewStringUTF((char *) result.mv_data);
  LOGI("result: %s", (char*)result.mv_data);
  mdb_txn_commit(txn);
  LOGI("get end");
  return jresult;
}
}
