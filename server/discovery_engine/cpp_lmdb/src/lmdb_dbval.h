//
//  lmdb_dbval.h
//  cpp_lmdb_c
//
//  Created by Erik van der Tier on 22/07/16.
//
//

#ifndef lmdb_dbval_h
#define lmdb_dbval_h
#include "external/lmdb/lmdb.h"
#include <string.h>

namespace LMDB {

struct DBVal {
  size_t mv_size; /**< size of the data item */
  void *mv_data;  /**< address of the data item */
  inline void setSize(size_t size) { mv_size = size; };
  inline size_t size() const { return mv_size; };
  inline void *buffer() const { return mv_data; };
  DBVal() {
    mv_size = 0;
    mv_data = nullptr;
  };
  DBVal(size_t size, void *data) {
    mv_size = size;
    mv_data = data;
  };
  template <typename V>
  DBVal(const V &src) {
    mv_size = src.size();
    mv_data = (void *)src.buffer();
  };
  DBVal(const MDB_val &src) {
    mv_size = src.mv_size;
    mv_data = src.mv_data;
  };
  DBVal(const uint64_t &src) {
    mv_size = sizeof(uint64_t);
    mv_data = (void *)new uint64_t(src);
  };
  DBVal(const uint32_t &src) {
    mv_size = sizeof(uint32_t);
    mv_data = (void *)new uint32_t(src);
  };
  DBVal(const uint16_t &src) {
    mv_size = sizeof(uint16_t);
    mv_data = (void *)new uint16_t(src);
  };
  DBVal(const uint8_t &src) {
    mv_size = sizeof(uint8_t);
    mv_data = (void *)new uint8_t(src);
  };
  DBVal(const int64_t &src) {
    mv_size = sizeof(int64_t);
    mv_data = (void *)new int64_t(src);
  };
  DBVal(const int16_t &src) {
    mv_size = sizeof(int16_t);
    mv_data = (void *)new int16_t(src);
  };
  DBVal(const int8_t &src) {
    mv_size = sizeof(int8_t);
    mv_data = (void *)new int8_t(src);
  };
  DBVal(const std::string &src) {
    mv_size = src.size() + 1;
    mv_data = (void *)strdup(src.c_str());
  };
  DBVal(const bool &src) {
    mv_size = sizeof(bool);
    mv_data = (void *)new bool(src);
  };
  DBVal(const int &src) {
    mv_size = sizeof(int);
    mv_data = (void *)new int(src);
  };
  DBVal(const double &src) {
    mv_size = sizeof(double);
    mv_data = (void *)new double(src);
  };
  DBVal(const float &src) {
    mv_size = sizeof(float);
    mv_data = (void *)new float(src);
  };
  DBVal(const char *src) {
    mv_size = strlen(src) + 1;
    mv_data = (void *)strdup(src);
  };
};

template <typename T>
auto makeDBVal(T val) -> DBVal {
  return DBVal(val);
};

template <typename T>
auto dbValAs(const DBVal &val) -> T {
  return static_cast<T>(T((char *)val.buffer()));
};

template <typename T>
auto dbValAsPtr(const DBVal &val) -> T * {
  if (val.mv_data == nullptr) return nullptr;
  return static_cast<T *>(val.buffer());
};

template <>
auto dbValAsPtr(const DBVal &val) -> DBVal *;

extern template auto dbValAsPtr(const DBVal &val) -> DBVal *;

template <>
auto dbValAs<uint64_t>(const DBVal &val) -> uint64_t;

extern template auto dbValAs<uint64_t>(const DBVal &val) -> uint64_t;

template <>
auto dbValAs<int64_t>(const DBVal &val) -> int64_t;

extern template auto dbValAs<int64_t>(const DBVal &val) -> int64_t;
}

#endif /* lmdb_dbval_h */
