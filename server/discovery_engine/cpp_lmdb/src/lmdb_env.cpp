//
//  lmdbEnv.cpp
//  libCortex
//
//  Created by Erik van der Tier on 28/08/14.
//  Copyright (c) 2014 Erik van der Tier BV. All rights reserved.
//

#include "lmdb_env.h"
#include "lmdb_database.h"

namespace LMDB {

template <>
auto dbValAs<uint64_t>(const DBVal &val) -> uint64_t {
  if (val.mv_data == nullptr) return 0;
  return *static_cast<uint64_t *>(val.buffer());
};

template <>
auto dbValAs<int64_t>(const DBVal &val) -> int64_t {
  if (val.mv_data == nullptr) return 0;
  return *static_cast<int64_t *>(val.buffer());
};

template <>
auto dbValAsPtr(const DBVal &val) -> DBVal * {
  return new DBVal(val);
};

using namespace XYError;

LMDBEnv::LMDBEnv(ErrorPtr &e) {
  initEnv(e, defaultDBPath, defaultDBMaxDBS, defaultDBMapSizeMB, defaultDBFlags,
          defaultDBMode);
}

LMDBEnv::LMDBEnv(ErrorPtr &e, MDBEnvPTR env) {
  env_ = env;
  dbConfig_ = std::make_shared<Database<DBVal, DBVal>>(
      e, *this, "DBConfig", static_cast<uint8_t>(MDB_CREATE));
}

LMDBEnv::LMDBEnv(ErrorPtr &e, const std::string &path, int maxDBS,
                 size_t mapSize, bool readOnly) {
  initEnv(e, path, maxDBS, mapSize,
          readOnly ? defaultDBFlags | uint8_t(MDB_RDONLY) : defaultDBFlags,
          defaultDBMode);
}

Transaction LMDBEnv::beginTransaction(ErrorPtr &e) const {
  return beginTransaction(e, 0);
}

Transaction LMDBEnv::beginTransaction(ErrorPtr &e, unsigned flags) const {
  ASSERT_ERROR_RESET(e);
  MDB_txn *txn = nullptr;
  int status = mdb_txn_begin(env_.get(), nullptr, flags, &txn);
  ON_MDB_ERROR_SET_CODE(e, status);
  return Transaction(txn, mdb_txn_abort);
}

Transaction LMDBEnv::beginTransaction(ErrorPtr &e, Transaction &txn) const {
  return beginTransaction(e, 0, txn);
}

void LMDBEnv::resetTransaction(ErrorPtr &e, Transaction &txn) const {
  ASSERT_ERROR_RESET(e);
  mdb_txn_reset(txn.get());
  int status = mdb_txn_renew(txn.get());
  ON_MDB_ERROR_SET_CODE(e, status);
}

Transaction LMDBEnv::beginTransaction(ErrorPtr &e, unsigned flags,
                                      Transaction &txn) const {
  ASSERT_ERROR_RESET(e);
  MDB_txn *nestedTxn = nullptr;

  int status = mdb_txn_begin(env_.get(), txn.get(), flags, &nestedTxn);
  ON_MDB_ERROR_SET_CODE(e, status);
  return Transaction(nestedTxn, mdb_txn_abort);
}

void LMDBEnv::commitTransaction(ErrorPtr &e, Transaction &txn) const {
  ASSERT_ERROR_RESET(e);
  int res = mdb_txn_commit(txn.get());
  txn.release();
  if (res) e.set(new error_t{res});
}

void LMDBEnv::setMaxID(ErrorPtr &e, const std::string &db, uint32_t id,
                       Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  auto dbName = ("MaxID_" + db);
  dbConfig_->add(e, dbName, id, txn, true);
  ON_ERROR_RETURN_CODE(e, -1);
}

void LMDBEnv::setMaxID(ErrorPtr &e, const std::string &db, uint32_t id) {
  ASSERT_ERROR_RESET(e);
  auto txn = beginTransaction(e);
  ON_ERROR_RETURN(e);
  setMaxID(e, db, id, txn);
  ON_ERROR_RETURN(e);
  commitTransaction(e, txn);
}

uint32_t LMDBEnv::getMaxID(ErrorPtr &e, const std::string &db,
                           Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  auto dbName = ("MaxID_" + db);
  auto MaxID = dbConfig_->get(e, dbName, txn);
  ON_ERROR_RETURN_CODE_VAL(e, -1, 0);
  return *static_cast<uint32_t *>(MaxID->buffer());
}

uint32_t LMDBEnv::getMaxID(ErrorPtr &e, const std::string &db) {
  ASSERT_ERROR_RESET(e);
  auto txn = beginTransaction(e);
  ON_ERROR_RETURN_VAL(e, 0);
  return getMaxID(e, db, txn);
}

MDBEnvPTR LMDBEnv::getLMDBEnv() const { return env_; }

void LMDBEnv::initEnv(ErrorPtr &e, const std::string &path, uint16_t maxDBS,
                      size_t mapSizeMB, uint8_t flags, mode_t mode) {
  ASSERT_ERROR_RESET(e);
  MDB_env *env = nullptr;
  int status;
  status = mdb_env_create(&env);
  ON_MDB_ERROR_RETURN(e, status);

  env_.reset(env, mdb_env_close);

  status = mdb_env_set_maxdbs(env_.get(), maxDBS);
  ON_MDB_ERROR_RETURN(e, status);

  status = mdb_env_set_mapsize(env_.get(), mapSizeMB * 1024 * 1024);
  ON_MDB_ERROR_RETURN(e, status);

  status = mdb_env_open(env_.get(), path.c_str(), flags, mode);
  ON_MDB_ERROR_RETURN(e, status);

  dbConfig_ = std::make_shared<Database<DBVal, DBVal>>(
      e, *this, "DBConfig", static_cast<uint8_t>(MDB_CREATE));
}

}  // namespace LMDB
