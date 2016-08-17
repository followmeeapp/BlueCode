//
//  xyPriorityQueue.h
//  PageRank
//
//  Created by Erik van der Tier on 29/01/15.
//  Copyright (c) 2015 Erik van der Tier BV. All rights reserved.
//

#ifndef XYGRAPHENGINE_XY_PRIORITY_QUEUE_H_
#define XYGRAPHENGINE_XY_PRIORITY_QUEUE_H_

#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>

using std::function;

namespace XYGraphEngine {

template <typename T>
class XYPriorityQueue {
 public:
  XYPriorityQueue(function<bool(const T &n1, const T &n2)> compare);

  void pop();
  T top();
  void push(const T &value);
  void update(const T &value);
  bool empty();
  size_t size();

 private:
  std::vector<T> _queue;
  function<bool(const T &n1, const T &n2)> _compare;
};

template <typename T>
XYPriorityQueue<T>::XYPriorityQueue(
    function<bool(const T &n1, const T &n2)> compare)
    : _queue(), _compare(compare) {
  _queue.reserve(50000);
}

template <typename T>
void XYPriorityQueue<T>::pop() {
  if (!_queue.empty()) {
    _queue.pop_back();
  }
}

template <typename T>
T XYPriorityQueue<T>::top() {
  if (!_queue.empty()) {
    return _queue.back();
  } else {
    static_assert(true, "Trying to top an empty queue!");
  }
}

template <typename T>
void XYPriorityQueue<T>::push(const T &value) {
  auto InsertPoint =
      std::find_if_not(begin(_queue), end(_queue),
                       [&value, this](T i) { return _compare(i, value); });
  _queue.insert(InsertPoint, value);
}

template <typename T>
void XYPriorityQueue<T>::update(const T &value) {
  auto OriginalElementIt = std::find_if(
      begin(_queue), end(_queue), [&value, this](T i) { return i == value; });
  if (OriginalElementIt != end(_queue)) {
    OriginalElementIt->prio = value.prio;
    std::sort(OriginalElementIt, end(_queue), _compare);
  } else {
    static_assert(true, "Value does not exist!");
  }
}

template <typename T>
bool XYPriorityQueue<T>::empty() {
  return _queue.empty();
}

template <typename T>
size_t XYPriorityQueue<T>::size() {
  return _queue.size();
}

}  // namespace PageRank

#endif
