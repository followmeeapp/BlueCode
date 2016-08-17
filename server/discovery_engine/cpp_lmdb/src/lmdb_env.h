//
//  lmdbEnv.h
//  libCortex
//
//  Created by Erik van der Tier on 28/08/14.
//  Copyright (c) 2014 Erik van der Tier BV. All rights reserved.
//

#ifndef CPP_LMDB_LMDB_ENV_H_
#define CPP_LMDB_LMDB_ENV_H_

#include <memory>
#include <map>
#include <string>
#include <functional>
#include <assert.h>
#include "lmdb_dbval.h"

#include "boost/any.hpp"
#include "boost/optional.hpp"
#include "external/lmdb/lmdb.h"

namespace LMDB {

struct error_t {
  int code = 0;
  virtual std::string message() { return mdb_strerror(code); };
  int getCode() { return code; };
  error_t(int code) { this->code = code; };
};

namespace XYError {
template <typename ET>
class Error {
 public:
  Error() : e_(nullptr){};
  Error(ET *e) : e_(e){};
  ~Error() { e_ = nullptr; };
  void set(ET *e) { e_ = e; };
  void reset() { e_ = nullptr; };
  operator bool() const { return e_ != nullptr ? e_->getCode() != 0 : false; };
  std::string message() const { return e_ != nullptr ? e_->message() : ""; }
  int code() const { return e_ != nullptr ? e_->getCode() : 0; };
  error_t *get() { return e_; };

 private:
  ET *e_;
};
}  // namespace XYError

using ErrorPtr = XYError::Error<error_t>;

const std::string defaultDBPath("/Users/erik/Documents/Suiron/Data/lmdb_test");
const uint16_t defaultDBMaxDBS = 32;
const size_t defaultDBMapSizeMB = 1024;
const uint8_t defaultDBFlags = 0;
const mode_t defaultDBMode = 0660;

using Transaction = std::unique_ptr<MDB_txn, void (*)(MDB_txn *)>;

template <typename K, typename V>
class Database;

template <typename K, typename V>
using DatabasePtr = std::shared_ptr<Database<K, V>>;

using MDBEnvPTR = std::shared_ptr<MDB_env>;
#define mdbEx(res) \
  if (res)         \
  throw std::runtime_error(std::string("LMDB Error: ") + mdb_strerror(res))

#define ON_MDB_ERROR_RETURN(e, res) \
  if (res) {                        \
    e.set(new LMDB::error_t(res));  \
    return;                         \
  }

#define ON_MDB_ERROR_SET_CODE(e, res) \
  if (res) {                          \
    e.set(new LMDB::error_t(res));    \
  }

#define ON_MDB_ERROR_RETURN_NONE(e, res) \
  if (res) {                             \
    e.set(new LMDB::error_t(res));       \
    return boost::none;                  \
  }

#define ON_MDB_ERROR_RETURN_VAL(e, res, val) \
  if (res) {                                 \
    e.set(new LMDB::error_t(res));           \
    return val;                              \
  }

#define ON_ERROR_RETURN(e) \
  if (e) return

#define ERROR_SET_CODE(e, code) e.set(new LMDB::error_t(code));

#define ERROR_RETURN_CODE(e, code)  \
  {                                 \
    e.set(new LMDB::error_t(code)); \
    return;                         \
  }

#define ON_ERROR_RETURN_VAL(e, val) \
  if (e) return val

#define ON_ERROR_RETURN_CODE_VAL(e, code, val) \
  if (e) {                                     \
    e.set(new LMDB::error_t(code));            \
    return val;                                \
  }

#define ON_ERROR_RETURN_CODE(e, code) \
  if (e) {                            \
    e.set(new LMDB::error_t(code));   \
    return;                           \
  }

#define ON_ERROR_SET_CODE(e, code) \
  if (e) e.set(new LMDB::error_t(code))

#define ON_ERROR_RETURN_NONE(e) \
  if (e) return boost::none

#define ERROR_RESET(e) e.reset()

#define ASSERT_ERROR_RESET(e) assert(!e.get())

class LMDBEnv {
 public:
  LMDBEnv(ErrorPtr &e);
  LMDBEnv(ErrorPtr &e, MDBEnvPTR env);
  LMDBEnv(ErrorPtr &e, const std::string &path, int maxDBS = defaultDBMaxDBS,
          size_t mapSizeMB = defaultDBMapSizeMB, bool readOnly = false);
  // virtual ~lmdbEnv();
  static char *getVersionString() {
    return mdb_version(nullptr, nullptr, nullptr);
  };
  Transaction beginTransaction(ErrorPtr &e) const;
  Transaction beginTransaction(ErrorPtr &e, unsigned flags) const;
  Transaction beginTransaction(ErrorPtr &e, Transaction &txn) const;
  Transaction beginTransaction(ErrorPtr &e, unsigned flags,
                               Transaction &txn) const;
  void commitTransaction(ErrorPtr &e, Transaction &txn) const;

  void resetTransaction(ErrorPtr &e, Transaction &txn) const;

  template <typename K, typename V>
  DatabasePtr<K, V> openDatabase(ErrorPtr &e, const std::string &name,
                                 uint8_t dbFlags);

  template <typename K, typename V>
  DatabasePtr<K, V> openDatabase(ErrorPtr &e, const std::string &name,
                                 uint8_t dbFlags, MDB_cmp_func *keyCmp,
                                 MDB_cmp_func *dupKeyCmp);

  template <typename K, typename V>
  DatabasePtr<K, V> openDatabase(ErrorPtr &e, const std::string &name,
                                 uint8_t dbFlags, MDB_cmp_func *keyCmp);

  template <typename K, typename V>
  void dropDatabase(ErrorPtr &e, const std::string &name);
  template <typename K, typename V>
  void dropDatabase(ErrorPtr &e, DatabasePtr<K, V> &db);
  template <typename K, typename V>
  void dropDatabase(ErrorPtr &e, const std::string &name, Transaction &txn);
  template <typename K, typename V>
  void dropDatabase(ErrorPtr &e, DatabasePtr<K, V> &db, Transaction &txn);

  void setMaxID(ErrorPtr &e, const std::string &db, uint32_t id);
  void setMaxID(ErrorPtr &e, const std::string &db, uint32_t id,
                Transaction &txn);

  uint32_t getMaxID(ErrorPtr &e, const std::string &db);
  uint32_t getMaxID(ErrorPtr &e, const std::string &db, Transaction &txn);

  MDBEnvPTR getLMDBEnv() const;

 private:
  MDBEnvPTR env_;
  DatabasePtr<DBVal, DBVal> dbConfig_;

  std::map<std::string, boost::any> dbList_;

  void initEnv(ErrorPtr &e, const std::string &path, uint16_t maxDBS,
               size_t mapSizeMB, uint8_t flags = defaultDBFlags,
               mode_t mode = defaultDBMode);
};

template <typename K, typename V>
DatabasePtr<K, V> LMDBEnv::openDatabase(ErrorPtr &e, const std::string &name,
                                        uint8_t dbFlags) {
  ASSERT_ERROR_RESET(e);
  if (dbList_.find(name) == dbList_.end()) {
    dbList_[name] = std::make_shared<Database<K, V>>(e, *this, name, dbFlags);
  }  // TODO: add error handling
  return boost::any_cast<DatabasePtr<K, V>>(dbList_[name]);
}

template <typename K, typename V>
DatabasePtr<K, V> LMDBEnv::openDatabase(ErrorPtr &e, const std::string &name,
                                        uint8_t dbFlags, MDB_cmp_func *keyCmp,
                                        MDB_cmp_func *dupKeyCmp) {
  ASSERT_ERROR_RESET(e);
  if (dbList_.find(name) == dbList_.end()) {
    dbList_[name] = std::make_shared<Database<K, V>>(e, *this, name, dbFlags,
                                                     keyCmp, dupKeyCmp);
  }  // TODO: add error handling
  return boost::any_cast<DatabasePtr<K, V>>(dbList_[name]);
}

template <typename K, typename V>
DatabasePtr<K, V> LMDBEnv::openDatabase(ErrorPtr &e, const std::string &name,
                                        uint8_t dbFlags, MDB_cmp_func *keyCmp) {
  ASSERT_ERROR_RESET(e);
  if (dbList_.find(name) == dbList_.end()) {
    dbList_[name] =
        std::make_shared<Database<K, V>>(e, *this, name, dbFlags, keyCmp);
  }  // TODO: add error handling
  return boost::any_cast<DatabasePtr<K, V>>(dbList_[name]);
}

template <typename K, typename V>
void LMDBEnv::dropDatabase(ErrorPtr &e, const std::string &name) {
  ASSERT_ERROR_RESET(e);
  auto txn = beginTransaction(e);
  ON_ERROR_RETURN(e);
  dropDatabase<K, V>(e, name, txn);
  ON_ERROR_RETURN(e);
  commitTransaction(e, txn);
}

template <typename K, typename V>
void LMDBEnv::dropDatabase(ErrorPtr &e, DatabasePtr<K, V> &db) {
  ASSERT_ERROR_RESET(e);
  auto txn = beginTransaction(e);
  ON_ERROR_RETURN(e);
  dropDatabase<K, V>(e, db, txn);
  ON_ERROR_RETURN(e);
  commitTransaction(e, txn);
}

template <typename K, typename V>
void LMDBEnv::dropDatabase(ErrorPtr &e, const std::string &name,
                           Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  if (dbList_.find(name) != dbList_.end()) {
    auto DropTxn = beginTransaction(e, txn);
    ON_ERROR_RETURN(e);
    auto DB = boost::any_cast<DatabasePtr<K, V>>(dbList_[name]);
    int status = mdb_drop(DropTxn.get(), DB->getDBI(), 1);
    ON_MDB_ERROR_RETURN(e, status);
    commitTransaction(e, DropTxn);
    dbList_.erase(name);
  }
}

template <typename K, typename V>
void LMDBEnv::dropDatabase(ErrorPtr &e, DatabasePtr<K, V> &db,
                           Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  auto DropTxn = beginTransaction(e, txn);
  ON_ERROR_RETURN(e);
  int status = mdb_drop(DropTxn.get(), db->getDBI(), 1);
  ON_MDB_ERROR_RETURN(e, status);
  commitTransaction(e, DropTxn);
  // TODO: need to fix the 'db registry' (dbList_) so it can be removed here
  // from that when needed
}

}  // namespace LMDB

#endif /* defined(CPP_LMDB_LMDB_ENV_H_) */
