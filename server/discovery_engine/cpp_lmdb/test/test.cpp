//
//  cpp_lmdb_test.cpp
//  cpp_lmdb
//
//  Created by Erik van der Tier on 06/02/15.
//
//

#include "../src/lmdb_env.h"
#include "../src/lmdb_database.h"
#include "../src/lmdb_cursor.h"
#include <assert.h>
#include "gtest/gtest.h"
#include <string>

namespace LMDB {
template <>
auto dbValAs<bool>(const DBVal &val) -> bool {
  return true;
};

template <>
auto makeDBVal<bool>(bool val) -> DBVal {
  return DBVal(sizeof(bool), static_cast<void *>(&val));
}

template <>
auto makeDBVal<std::string &>(std::string &val) -> DBVal {
  return DBVal(val.size() + 1, static_cast<void *>(strdup(val.c_str())));
}

}  // namespace LMDB

namespace {
using namespace LMDB;

struct val {
  int64_t time;
  int64_t id;
};

auto InfoCmp(const MDB_val *val1, const MDB_val *val2) -> int {
  auto v1 = *static_cast<val *>(val1->mv_data);
  auto v2 = *static_cast<val *>(val2->mv_data);
  // sorting from high to low
  if (v1.time < v2.time)
    return 1;
  else if (v1.time > v2.time)
    return -1;
  else {  // sorting id's normal order
    if (v1.id < v2.id)
      return -1;
    else if (v1.id > v2.id)
      return 1;
  }
  return 0;
}

TEST(cpp_lmdb_tests, Initialize) {
  ErrorPtr e;

  auto TestEnv = LMDBEnv(e);

  std::cout << TestEnv.getVersionString();
  if (e) std::cout << e.get()->message();
  ASSERT_FALSE(e);
  TestEnv.setMaxID(e, "TestDB", (uint32_t)1);

  auto StringDB = TestEnv.openDatabase<DBVal, DBVal>(e, "StringDB", 0);
  if (auto txn = TestEnv.beginTransaction(e)) {
    ASSERT_EQ(1, TestEnv.getMaxID(e, "TestDB", txn));
    auto keyName = ("TestDB" + std::string("test"));
    StringDB->add(e, keyName, 10, txn, true);
    auto IntOut = (int *)StringDB->get(e, keyName, txn)->buffer();
    if (IntOut) std::cout << *IntOut << std::endl;
    TestEnv.commitTransaction(e, txn);
  }
  auto TestDB = TestEnv.openDatabase<DBVal, DBVal>(e, "TestDB", 0);

  TestDB->add(e, 1, "testdata1", true);
  TestDB->add(e, 2, "testdata2", true);

  if (auto txn = TestEnv.beginTransaction(e)) {
    // ERROR_RESET(e);
    auto out1 = (char *)TestDB->get(e, 1, txn)->buffer();
    auto out2 = (char *)TestDB->get(e, 2, txn)->buffer();
    ASSERT_TRUE(out1);
    if (out1) ASSERT_TRUE(strcmp("testdata1", out1) == 0);
    std::cout << out1 << std::endl;
    ASSERT_TRUE(out2);
    if (out2) ASSERT_TRUE(strcmp("testdata2", out2) == 0);
    TestEnv.commitTransaction(e, txn);
  }

  auto intVal = makeDBVal<bool>(true);
  auto retVal = dbValAs<bool>(intVal);
  ASSERT_TRUE(retVal);

  auto strVal = makeDBVal<std::string>("hello");
  auto retStrVal = dbValAs<std::string>(strVal);
  ASSERT_EQ("hello", retStrVal);

  if (auto txn = TestEnv.beginTransaction(e)) {
    {
      auto TestNestedCursor = TestDB->getCursor(e, txn);
      if (e) std::cout << e.get()->message();
      ASSERT_FALSE(e);
    }
    ERROR_RESET(e);
    TestEnv.commitTransaction(e, txn);
    // e.set(new error_t(3));
  }
}
TEST(cpp_lmdb_tests, Dup) {
  ErrorPtr e;
  auto TestEnv = LMDBEnv(e);
  if (e) std::cout << e.get()->message();
  ASSERT_FALSE(e);

  auto DupDB = TestEnv.openDatabase<DBVal, DBVal>(
      e, "recentCards", MDB_DUPSORT | MDB_DUPFIXED | MDB_INTEGERKEY);
  DupDB->setDubKeyCompareFunction(e, InfoCmp);

  auto txn = TestEnv.beginTransaction(e);
  val testVal1 = {1, 1};
  val testVal2 = {2, 2};
  val testVal3 = {3, 3};
  val testVal4 = {4, 1};
  val testVal5 = {4, 3};
  val testVal6 = {4, 4};

  DupDB->add(e, 1, DBVal(sizeof(val), (void *)&testVal1), txn, true);
  DupDB->add(e, 1, DBVal(sizeof(val), (void *)&testVal2), txn, true);
  DupDB->add(e, 1, DBVal(sizeof(val), (void *)&testVal3), txn, true);
  DupDB->add(e, 1, DBVal(sizeof(val), (void *)&testVal4), txn, true);
  DupDB->add(e, 1, DBVal(sizeof(val), (void *)&testVal5), txn, true);
  DupDB->add(e, 1, DBVal(sizeof(val), (void *)&testVal6), txn, true);

  auto cur = DupDB->getCursor(e, txn);
  auto res = cur->getDup(e, 1, DBVal(sizeof(val), (void *)&testVal3));
  if (res.is_initialized())
    std::cout << "yeah" << (*static_cast<val *>(res->value.buffer())).id
              << std::endl;
  res = cur->getNextDup(e, 1);
  if (res.is_initialized())
    std::cout << "yeah" << (*static_cast<val *>(res->value.buffer())).id
              << std::endl;

  val testDubVal = {4, 5};
  auto GTRes =
      cur->getDupKeyGreaterEqual(e, 1, DBVal(sizeof(val), (void *)&testDubVal));
  if (GTRes.is_initialized())
    std::cout << "yeah, dubkeyGT"
              << (*static_cast<val *>(GTRes->value.buffer())).time << std::endl;
}
}
