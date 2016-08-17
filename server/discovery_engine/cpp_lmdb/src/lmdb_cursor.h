//
//  lmdbCursor.h
//  libCortex
//
//  Created by Erik van der Tier on 29/08/14.
//  Copyright (c) 2014 Erik van der Tier BV. All rights reserved.
//

#ifndef CPP_LMDB_LMDB_CURSOR_H_
#define CPP_LMDB_LMDB_CURSOR_H_

#include <iostream>
#include <memory>
#include <cmath>
#include "lmdb_database.h"

namespace LMDB {

template <typename K, typename V>
using OptKeyValuePtr = boost::optional<key_value_t<K, V>>;

template <typename K, typename V>
class Cursor {
 public:
  Cursor(ErrorPtr& e, Database<K, V>& db, Transaction& txn);
  virtual ~Cursor();

  LMDBEnv& getEnv();

  boost::optional<size_t> getCount(ErrorPtr& e);

  Transaction& getTransaction(ErrorPtr& e);

  OptKeyValuePtr<K, V> getKeyLessEqual(ErrorPtr& e, const K& key);

  OptKeyValuePtr<K, V> getDupKeyLessEqual(ErrorPtr& e, const V& dupKey,
                                          const V& value);

  OptKeyValuePtr<K, V> getDupKeyGreaterEqual(ErrorPtr& e, const V& dupKey,
                                             const V& value);

  OptKeyValuePtr<K, V> getKeyGreaterEqual(ErrorPtr& e, const K& key);

  OptKeyValuePtr<K, V> getPrevious(ErrorPtr& e);

  OptKeyValuePtr<K, V> getPreviousDup(ErrorPtr& e, const K& key);

  OptKeyValuePtr<K, V> getPreviousNoDup(ErrorPtr& e, const K& key);

  OptKeyValuePtr<K, V> getFirst(ErrorPtr& e);
  OptKeyValuePtr<K, V> getDup(ErrorPtr& e, const K& key, const V& value);
  OptKeyValuePtr<K, V> getNext(ErrorPtr& e);
  OptKeyValuePtr<K, V> getLast(ErrorPtr& e);

  void appendDupValue(ErrorPtr& e, const K& key, const V& value);
  OptKeyValuePtr<K, V> getFirstDup(ErrorPtr& e, const K& key);
  void setFirstDup(ErrorPtr& e, const K& key);

  OptKeyValuePtr<K, V> getNextDup(ErrorPtr& e, const K& key);

  OptKeyValuePtr<K, V> getFirstMultiple(ErrorPtr& e);
  OptKeyValuePtr<K, V> getNextMultiple(ErrorPtr& e);

 private:
  Database<K, V>& db_;
  LMDBEnv& env_;
  MDB_cmp_func* keyCmp_;
  MDB_cmp_func* dubKeyCmp_;
  Transaction& txn_;
  MDB_cursor* lmdbCursor_;

  inline OptKeyValuePtr<K, V> cursorOp(ErrorPtr& e, const K& key,
                                       MDB_cursor_op op);
  inline OptKeyValuePtr<K, V> cursorOp(ErrorPtr& e, const K& key,
                                       const V& value, MDB_cursor_op op);
  inline OptKeyValuePtr<K, V> cursorOp(ErrorPtr& e, MDB_cursor_op op);
};

template <typename K, typename V>
Cursor<K, V>::Cursor(ErrorPtr& e, Database<K, V>& db, Transaction& txn)
    : db_(db), env_(db.getEnv()), txn_(txn) {
  ON_ERROR_RETURN(e);
  keyCmp_ = db.getKeyCompareFunction();
  dubKeyCmp_ = db.getDubKeyCompareFunction();
  ERROR_RESET(e);
  int status = mdb_cursor_open(txn_.get(), db_.getDBI(), &lmdbCursor_);
  ON_MDB_ERROR_RETURN(e, status);
}

template <typename K, typename V>
Cursor<K, V>::~Cursor() {
  if (lmdbCursor_) {
    mdb_cursor_close(lmdbCursor_);
    lmdbCursor_ = nullptr;
    ErrorPtr e;
  }
}

template <typename K, typename V>
boost::optional<size_t> Cursor<K, V>::getCount(ErrorPtr& e) {
  ASSERT_ERROR_RESET(e);
  size_t size = 0;

  auto status = mdb_cursor_count(lmdbCursor_, &size);
  if (status == MDB_NOTFOUND)
    size = 0;
  else
    ON_MDB_ERROR_RETURN_NONE(e, status);

  return boost::optional<size_t>(size);
}

template <typename K, typename V>
Transaction& Cursor<K, V>::getTransaction(ErrorPtr& e) {
  ASSERT_ERROR_RESET(e);
  return txn_;
}

template <typename K, typename V>
LMDBEnv& Cursor<K, V>::getEnv() {
  return env_;
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getKeyLessEqual(ErrorPtr& e, const K& key) {
  ASSERT_ERROR_RESET(e);

  // 'save' the key value in searchKey val so we can compare to the keys that
  // are found in the DB. As the variables are references the value will get
  // modified by LMDB.

  K searchKeyVal = key;
  MDB_val searchKey = {searchKeyVal.mv_size, searchKeyVal.mv_data};
  MDB_val mdbKey = {key.mv_size, key.mv_data};
  MDB_val mdbVal;

  // first search for the lowest key that is greater or equal to key.
  int status = mdb_cursor_get(lmdbCursor_, &mdbKey, &mdbVal, MDB_SET_RANGE);
  if (status == MDB_NOTFOUND) {
    // if no such key is found our searchkey is higher than any key in the DB,
    // so we select the LAST key/value.

    status = mdb_cursor_get(lmdbCursor_, &mdbKey, &mdbVal, MDB_LAST);
    if (status != MDB_NOTFOUND) ON_MDB_ERROR_RETURN_NONE(e, status);

  } else if (keyCmp_(&mdbKey, &searchKey) != 0) {
    // the key returned by LMDB is unequal to the original searchKey it is the
    // first higher key and we select the previous key/value, which will be
    // lower.

    status = mdb_cursor_get(lmdbCursor_, &mdbKey, &mdbVal, MDB_PREV);
    if (status != MDB_NOTFOUND) ON_MDB_ERROR_RETURN_NONE(e, status);
  }
  if (status == MDB_NOTFOUND) return boost::none;
  // we can just cast the values to K and V pointers and assign those.
  return OptKeyValuePtr<K, V>({DBVal(mdbKey), DBVal(mdbVal)});
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getDupKeyLessEqual(ErrorPtr& e,
                                                      const V& dupKey,
                                                      const V& value) {
  ASSERT_ERROR_RESET(e);
  MDB_val mdbKey;
  MDB_val mdbCurrentMultiple;
  size_t numVals = 0;
  size_t size = sizeof(V);
  bool notFound = true;

  // mdb internal format versions of the dup key and dup value
  MDB_val mdbDupKey = {dupKey.mv_size, dupKey.mv_data};
  MDB_val mdbDupVal;

  int firstValNextMultipleCmp = 0;
  MDB_val mdbNextMultiple;

  int status = mdb_cursor_count(lmdbCursor_, &numVals);
  ON_MDB_ERROR_RETURN_NONE(e, status);

  // set the cursor to the first dup for the key and get the first page of
  // multiples. mdbKey is a dummy key, which isn't used with the cursor ops
  // we're using right now.
  status =
      mdb_cursor_get(lmdbCursor_, &mdbKey, &mdbCurrentMultiple, MDB_FIRST_DUP);
  if (status == MDB_NOTFOUND) return boost::none;
  ON_MDB_ERROR_RETURN_NONE(e, status);

  status = mdb_cursor_get(lmdbCursor_, &mdbKey, &mdbCurrentMultiple,
                          MDB_GET_MULTIPLE);
  if (status == MDB_NOTFOUND) return boost::none;
  ON_MDB_ERROR_RETURN_NONE(e, status);

  // scan forward through possible pages (multiples) of values to find the one
  // we'll search in. Each time we test a 'nextMultiple'.
  while (mdb_cursor_get(lmdbCursor_, &mdbKey, &mdbNextMultiple,
                        MDB_NEXT_MULTIPLE) == 0) {
    // compage the first key in the next page with the dupkey.

    firstValNextMultipleCmp = dubKeyCmp_(&mdbNextMultiple, &mdbDupKey);

    if (firstValNextMultipleCmp > 0) {
      // the first value of the next multiple is higher than the key so the
      // value cannot be found there, search the current page.

      break;

    } else if (firstValNextMultipleCmp < 0) {
      // the first value in the next page is less than key, so this page or a
      // possible next page is where we search further.
      // the currentMultiple is updated to the 'nextMultiple' before we look for
      // the next 'next'.

      mdbCurrentMultiple = mdbNextMultiple;

    } else {
      // the first value of the next page is the key, we're done

      mdbDupVal.mv_data = mdbNextMultiple.mv_data;
      notFound = false;
      break;
    }
  }

  if (notFound) {
    // we have a page in which the value could be found, do a binary search in
    // this page

    uint64_t numValsInPage = mdbCurrentMultiple.mv_size / size;
    int dupValCmp = 0;

    int64_t first = 0;
    int64_t last = numValsInPage - 1;
    int64_t middle = 0;

    while (first <= last) {
      middle = (first + last) / 2;

      mdbDupVal.mv_data =
          (void*)((char*)(mdbCurrentMultiple.mv_data) + size * middle);
      dupValCmp = dubKeyCmp_(&mdbDupVal, &mdbDupKey);

      if (dupValCmp < 0)
        first = middle + 1;
      else if (dupValCmp > 0)
        last = middle - 1;
      else
        break;
    }

    if (last < 0) {
      return boost::none;
      // ON_MDB_ERROR_RETURN_NONE(e, MDB_NOTFOUND);
    }

    if (dupValCmp != 0) {
      mdbDupVal.mv_data =
          (void*)((char*)(mdbCurrentMultiple.mv_data) + size * last);
    }
  }
  return OptKeyValuePtr<K, V>{DBVal(mdbKey), DBVal(mdbDupVal)};
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getDupKeyGreaterEqual(ErrorPtr& e,
                                                         const V& dupKey,
                                                         const V& value) {
  auto Candidate = cursorOp(e, dupKey, value, MDB_GET_BOTH_RANGE);
  if (!Candidate.is_initialized()) return boost::none;
  auto Cmp = dubKeyCmp_((MDB_val*)&(Candidate->value), (MDB_val*)&value);
  if (Cmp < 0) Candidate = getNextDup(e, dupKey);
  if (!Candidate.is_initialized()) return boost::none;
  return Candidate;
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getKeyGreaterEqual(ErrorPtr& e,
                                                      const K& key) {
  return cursorOp(e, key, MDB_SET_RANGE);
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getPrevious(ErrorPtr& e) {
  return cursorOp(e, MDB_PREV);
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getPreviousDup(ErrorPtr& e, const K& key) {
  return cursorOp(e, key, MDB_PREV_DUP);
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getPreviousNoDup(ErrorPtr& e, const K& key) {
  return cursorOp(e, key, MDB_PREV_NODUP);
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getNext(ErrorPtr& e) {
  return cursorOp(e, MDB_NEXT);
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getDup(ErrorPtr& e, const K& key,
                                          const V& value) {
  return cursorOp(e, key, value, MDB_GET_BOTH);
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getFirst(ErrorPtr& e) {
  return cursorOp(e, MDB_FIRST);
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getLast(ErrorPtr& e) {
  return cursorOp(e, MDB_LAST);
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getFirstDup(ErrorPtr& e, const K& key) {
  return cursorOp(e, key, MDB_SET_KEY);
}

template <typename K, typename V>
void Cursor<K, V>::setFirstDup(ErrorPtr& e, const K& key) {
  ASSERT_ERROR_RESET(e);
  auto Key = key;
  DBVal mdbKey = makeDBVal(Key);
  MDB_val mdbVal;
  int status =
      mdb_cursor_get(lmdbCursor_, (MDB_val*)&mdbKey, &mdbVal, MDB_SET_KEY);
  ON_MDB_ERROR_RETURN(e, status);
}

template <typename K, typename V>
void Cursor<K, V>::appendDupValue(ErrorPtr& e, const K& key, const V& value) {
  ASSERT_ERROR_RESET(e);
  auto Key = key;
  DBVal mdbKey = makeDBVal(Key);
  MDB_val mdbVal;
  int status =
      mdb_cursor_put(lmdbCursor_, (MDB_val*)&mdbKey, &mdbVal, MDB_APPENDDUP);
  ON_MDB_ERROR_RETURN(e, status);
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getNextDup(ErrorPtr& e, const K& key) {
  return cursorOp(e, key, MDB_NEXT_DUP);
}

template <typename K, typename V>
inline OptKeyValuePtr<K, V> Cursor<K, V>::cursorOp(ErrorPtr& e, const K& key,
                                                   MDB_cursor_op op) {
  ASSERT_ERROR_RESET(e);
  auto Key = key;
  DBVal mdbKey = makeDBVal(Key);
  MDB_val mdbVal;
  int status = mdb_cursor_get(lmdbCursor_, (MDB_val*)&mdbKey, &mdbVal, op);
  if (status == MDB_NOTFOUND) return boost::none;
  ON_MDB_ERROR_RETURN_NONE(e, status);
  return OptKeyValuePtr<K, V>{{DBVal(mdbKey), DBVal(mdbVal)}};
}

template <typename K, typename V>
inline OptKeyValuePtr<K, V> Cursor<K, V>::cursorOp(ErrorPtr& e, const K& key,
                                                   const V& value,
                                                   MDB_cursor_op op) {
  ASSERT_ERROR_RESET(e);
  auto Key = key;
  DBVal mdbKey = makeDBVal(Key);
  DBVal mdbVal = makeDBVal(value);
  int status =
      mdb_cursor_get(lmdbCursor_, (MDB_val*)&mdbKey, (MDB_val*)&mdbVal, op);
  if (status == MDB_NOTFOUND) return boost::none;
  ON_MDB_ERROR_RETURN_NONE(e, status);
  return OptKeyValuePtr<K, V>{{DBVal(mdbKey), DBVal(mdbVal)}};
}

template <typename K, typename V>
inline OptKeyValuePtr<K, V> Cursor<K, V>::cursorOp(ErrorPtr& e,
                                                   MDB_cursor_op op) {
  ASSERT_ERROR_RESET(e);
  MDB_val mdbKey;
  MDB_val mdbVal;
  int status = mdb_cursor_get(lmdbCursor_, &mdbKey, &mdbVal, op);
  if (status == MDB_NOTFOUND) return boost::none;
  ON_MDB_ERROR_RETURN_NONE(e, status);
  return OptKeyValuePtr<K, V>{{DBVal(mdbKey), DBVal(mdbVal)}};
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getFirstMultiple(ErrorPtr& e) {
  return cursorOp(e, MDB_GET_MULTIPLE);
}

template <typename K, typename V>
OptKeyValuePtr<K, V> Cursor<K, V>::getNextMultiple(ErrorPtr& e) {
  return cursorOp(e, MDB_NEXT_MULTIPLE);
}

}  // namespace LMDB

#endif /* defined(CPP_LMDB_LMDB_CURSOR_H_) */
