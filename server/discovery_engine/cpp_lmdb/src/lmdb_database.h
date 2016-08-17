//
//  lmdbDatabase.h
//  libCortex
//
//  Created by Erik van der Tier on 28/08/14.
//  Copyright (c) 2014 Erik van der Tier BV. All rights reserved.
//

#ifndef CPP_LMDB_LMDB_DATABASE_H_
#define CPP_LMDB_LMDB_DATABASE_H_

#include <iostream>
#include "lmdb_env.h"
#include "boost/optional.hpp"

namespace LMDB {

template <typename K, typename V>
class Cursor;

template <typename K, typename V>
using CursorPtr = std::unique_ptr<Cursor<K, V>>;

template <typename K, typename V>
struct key_value_t {
  K key;
  V value;
};

template <typename K, typename V>
class Database {
 public:
  Database(ErrorPtr& e, LMDBEnv& env, const std::string& name, uint8_t dbFlags);

  Database(ErrorPtr& e, LMDBEnv& env, const std::string& name, uint8_t dbFlags,
           MDB_cmp_func* keyCmp);

  Database(ErrorPtr& e, LMDBEnv& env, const std::string& name, uint8_t dbFlags,
           MDB_cmp_func* keyCmp, MDB_cmp_func* dupCmp);

  virtual ~Database();

  void setKeyCompareFunction(ErrorPtr& e, MDB_cmp_func* compare);
  void setKeyCompareFunction(ErrorPtr& e, MDB_cmp_func* compare,
                             Transaction& txn);

  MDB_cmp_func* getKeyCompareFunction() const;

  void setDubKeyCompareFunction(ErrorPtr& e, MDB_cmp_func* compare);
  void setDubKeyCompareFunction(ErrorPtr& e, MDB_cmp_func* compare,
                                Transaction& txn);

  MDB_cmp_func* getDubKeyCompareFunction() const;

  void add(ErrorPtr& e, const K& key, const V& value, Transaction& txn,
           bool allowUpdate);

  void add(ErrorPtr& e, const K& key, const V& value, bool allowUpdate);

  // void get(const K& key, V& value, Transaction& txn) const;
  V* get(ErrorPtr& e, const K& key, Transaction& txn) const;
  boost::optional<V> getCopy(ErrorPtr& e, const K& key, Transaction& txn) const;
  boost::optional<V> getCopy(ErrorPtr& e, const K& key) const;

  void del(ErrorPtr& e, const K& key, Transaction& txn);
  void del(ErrorPtr& e, const K& key, const K& val, Transaction& txn);
  void del(ErrorPtr& e, const K& key);

  void clear(ErrorPtr& e, Transaction& txn);
  void clear(ErrorPtr& e);

  CursorPtr<K, V> getCursor(ErrorPtr& e, Transaction& txn);

  LMDBEnv& getEnv();
  MDB_dbi getDBI();

 private:
  std::string Name_;
  MDB_cmp_func* keyCmp_;
  MDB_cmp_func* dubKeyCmp_;
  int noUpdateFlag_ = 0;
  LMDBEnv& env_;
  MDB_dbi dbI_ = 0;
};

template <typename K, typename V>
Database<K, V>::Database(ErrorPtr& e, LMDBEnv& env, const std::string& name,
                         uint8_t dbFlags)
    : Name_(name), env_(env) {
  ASSERT_ERROR_RESET(e);
  auto txn = env_.beginTransaction(e);
  ON_ERROR_RETURN(e);
  unsigned flags = dbFlags | MDB_CREATE;

  noUpdateFlag_ = dbFlags & MDB_DUPSORT ? MDB_NODUPDATA : MDB_NOOVERWRITE;

  int status = mdb_dbi_open(txn.get(), name.c_str(), flags, &dbI_);
  ON_MDB_ERROR_RETURN(e, status);
  env_.commitTransaction(e, txn);
}

template <typename K, typename V>
Database<K, V>::Database(ErrorPtr& e, LMDBEnv& env, const std::string& name,
                         uint8_t dbFlags, MDB_cmp_func* keyCmp)
    : Name_(name), keyCmp_(keyCmp), env_(env) {
  ASSERT_ERROR_RESET(e);
  auto txn = env_.beginTransaction(e);
  ON_ERROR_RETURN(e);
  unsigned flags = dbFlags | MDB_CREATE;
  noUpdateFlag_ = dbFlags & MDB_DUPSORT ? MDB_NODUPDATA : MDB_NOOVERWRITE;

  int status = mdb_dbi_open(txn.get(), name.c_str(), flags, &dbI_);
  ON_MDB_ERROR_RETURN(e, status);

  setKeyCompareFunction(e, keyCmp, txn);
  ON_ERROR_RETURN(e);

  env_.commitTransaction(e, txn);
}

template <typename K, typename V>
Database<K, V>::Database(ErrorPtr& e, LMDBEnv& env, const std::string& name,
                         uint8_t dbFlags, MDB_cmp_func* keyCmp,
                         MDB_cmp_func* dupCmp)
    : keyCmp_(keyCmp), dubKeyCmp_(dupCmp), env_(env) {
  ASSERT_ERROR_RESET(e);
  auto txn = env_.beginTransaction(e);
  ON_ERROR_RETURN(0);
  unsigned flags = dbFlags | MDB_CREATE;
  noUpdateFlag_ = dbFlags & MDB_DUPSORT ? MDB_NODUPDATA : MDB_NOOVERWRITE;

  int status = mdb_open(txn.get(), name.c_str(), flags, &dbI_);
  ON_MDB_ERROR_RETURN(e, status);

  setKeyCompareFunction(e, keyCmp, txn);
  ON_ERROR_RETURN(0);

  setDubKeyCompareFunction(e, dupCmp, txn);
  ON_ERROR_RETURN(0);
  env_.commitTransaction(e, txn);
}

template <typename K, typename V>
Database<K, V>::~Database() {
  mdb_dbi_close(env_.getLMDBEnv().get(), dbI_);
}

template <typename K, typename V>
void Database<K, V>::setKeyCompareFunction(ErrorPtr& e, MDB_cmp_func* compare) {
  ASSERT_ERROR_RESET(e);
  auto txn = env_.beginTransaction(e);
  ON_ERROR_RETURN(e);
  setDubKeyCompareFunction(e, compare, txn);
  ON_ERROR_RETURN(e);

  env_.commitTransaction(e, txn);
}

template <typename K, typename V>
void Database<K, V>::setKeyCompareFunction(ErrorPtr& e, MDB_cmp_func* compare,
                                           Transaction& txn) {
  ASSERT_ERROR_RESET(e);
  keyCmp_ = compare;
  int status = mdb_set_compare(txn.get(), dbI_, keyCmp_);
  ON_MDB_ERROR_RETURN(e, status);
}

template <typename K, typename V>
MDB_cmp_func* Database<K, V>::getKeyCompareFunction() const {
  return keyCmp_;
}

template <typename K, typename V>
void Database<K, V>::setDubKeyCompareFunction(ErrorPtr& e,
                                              MDB_cmp_func* compare) {
  ASSERT_ERROR_RESET(e);
  auto txn = env_.beginTransaction(e);
  ON_ERROR_RETURN(e);
  setDubKeyCompareFunction(e, compare, txn);
  ON_ERROR_RETURN(e);
  env_.commitTransaction(e, txn);
}

template <typename K, typename V>
void Database<K, V>::setDubKeyCompareFunction(ErrorPtr& e,
                                              MDB_cmp_func* compare,
                                              Transaction& txn) {
  ASSERT_ERROR_RESET(e);
  dubKeyCmp_ = compare;
  int status = mdb_set_dupsort(txn.get(), dbI_, dubKeyCmp_);
  ON_MDB_ERROR_RETURN(e, status);
}

template <typename K, typename V>
MDB_cmp_func* Database<K, V>::getDubKeyCompareFunction() const {
  return dubKeyCmp_;
}

template <typename K, typename V>
void Database<K, V>::add(ErrorPtr& e, const K& key, const V& value,
                         Transaction& txn, bool allowUpdate) {
  ASSERT_ERROR_RESET(e);
  DBVal mdbKey = DBVal(key);
  DBVal mdbVal = DBVal(value);

  int status = mdb_put(txn.get(), dbI_, (MDB_val*)&mdbKey, (MDB_val*)&mdbVal,
                       allowUpdate ? 0 : noUpdateFlag_);
  ON_MDB_ERROR_RETURN(e, status);
}

template <typename K, typename V>
void Database<K, V>::add(ErrorPtr& e, const K& key, const V& value,
                         bool allowUpdate) {
  ASSERT_ERROR_RESET(e);
  auto txn = env_.beginTransaction(e);
  ON_ERROR_RETURN(e);
  add(e, key, value, txn, allowUpdate);
  ON_ERROR_RETURN(e);
  env_.commitTransaction(e, txn);

  // we don't need to explicitely abort the transaction as that happens
  // automatically when txn goes out of scope
}

template <typename K, typename V>
V* Database<K, V>::get(ErrorPtr& e, const K& key, Transaction& txn) const {
  ASSERT_ERROR_RESET(e);
  DBVal mdbKey = makeDBVal<K>(key);  // DBVal(key);
  MDB_val mdbVal;

  int status = mdb_get(txn.get(), dbI_, (MDB_val*)&mdbKey, &mdbVal);
  // NOT_FOUND is not an error
  // NOT_FOUND means the result is nullptr
  if (status == MDB_NOTFOUND) return nullptr;
  ON_MDB_ERROR_RETURN_VAL(e, status, nullptr);
  return dbValAsPtr<V>(mdbVal);
}

template <typename K, typename V>
boost::optional<V> Database<K, V>::getCopy(ErrorPtr& e, const K& key,
                                           Transaction& txn) const {
  ASSERT_ERROR_RESET(e);
  if (auto ValuePtr = get(e, key, txn)) {
    return boost::optional<V>(*ValuePtr);
  }
  // NOT_FOUND is not an error
  // NOT_FOUND means the optional will be undefined
  if (e.code() == MDB_NOTFOUND) ERROR_RESET(e);
  return boost::none;
}

template <typename K, typename V>
boost::optional<V> Database<K, V>::getCopy(ErrorPtr& e, const K& key) const {
  ASSERT_ERROR_RESET(e);
  auto txn = env_.beginTransaction(e);
  ON_ERROR_RETURN_VAL(e, boost::none);

  boost::optional<V> result;
  result = getCopy(e, key, txn);
  ON_ERROR_RETURN_NONE(e);

  env_.commitTransaction(e, txn);
  ON_ERROR_RETURN_NONE(e);
  return result;
}

template <typename K, typename V>
void Database<K, V>::del(ErrorPtr& e, const K& key, Transaction& txn) {
  ASSERT_ERROR_RESET(e);
  MDB_val mdbKey = {key.mv_size, key.mv_data};
  MDB_val* mdbVal = nullptr;

  int status = mdb_del(txn.get(), dbI_, &mdbKey, mdbVal);
  ON_MDB_ERROR_RETURN(e, status);
}

template <typename K, typename V>
void Database<K, V>::del(ErrorPtr& e, const K& key, const K& val,
                         Transaction& txn) {
  ASSERT_ERROR_RESET(e);
  DBVal mdbKey = makeDBVal<K>(key);
  DBVal mdbVal = makeDBVal<V>(val);

  int status = mdb_del(txn.get(), dbI_, (MDB_val*)&mdbKey, (MDB_val*)&mdbVal);
  ON_MDB_ERROR_RETURN(e, status);
}

template <typename K, typename V>
void Database<K, V>::del(ErrorPtr& e, const K& key) {
  ASSERT_ERROR_RESET(e);
  auto txn = env_.beginTransaction(e);
  ON_ERROR_RETURN(e);
  del(e, key, txn);
  ON_ERROR_RETURN(e);
  env_.commitTransaction(e, txn);
}

template <typename K, typename V>
void Database<K, V>::clear(ErrorPtr& e, Transaction& txn) {
  ASSERT_ERROR_RESET(e);

  int status = mdb_drop(txn.get(), dbI_, 0);
  ON_MDB_ERROR_RETURN(e, status);
}

template <typename K, typename V>
void Database<K, V>::clear(ErrorPtr& e) {
  ASSERT_ERROR_RESET(e);
  auto txn = env_.beginTransaction(e);
  ON_ERROR_RETURN(e);
  clear(txn, e);
  ON_ERROR_RETURN(e);
  env_.commitTransaction(e, txn);
}

template <typename K, typename V>
CursorPtr<K, V> Database<K, V>::getCursor(ErrorPtr& e, Transaction& txn) {
  ASSERT_ERROR_RESET(e);
  return CursorPtr<K, V>(new Cursor<K, V>(e, *this, txn));
}

template <typename K, typename V>
LMDBEnv& Database<K, V>::getEnv() {
  return env_;
}

template <typename K, typename V>
MDB_dbi Database<K, V>::getDBI() {
  return dbI_;
}

}  // namespace LMDB

#endif /* defined(CPP_LMDB_LMDB_DATABASE_H_) */