//
//  XYPriorityQueueTest.cpp
//  PageRank
//
//  Created by Erik van der Tier on 29/01/15.
//  Copyright (c) 2015 Erik van der Tier BV. All rights reserved.
//

#include "../xy_priority_queue.h"
#include <gtest/gtest.h>

using namespace XYGraphEngine;

namespace {

// The fixture for testing class Foo.
class XYPriorityQueueTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  XYPriorityQueueTest() {
    // You can do set-up work for each test here.
  }

  virtual ~XYPriorityQueueTest() {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  // Objects declared here can be used by all tests in the test case for Foo.
};

TEST_F(XYPriorityQueueTest, QueueOfInts) {
  XYPriorityQueue<int> testQueue(
      [](const int &n1, const int &n2) { return n1 < n2; });

  testQueue.push(1);
  testQueue.push(2);
  testQueue.push(3);

  ASSERT_EQ(3, testQueue.top());
  testQueue.pop();
  ASSERT_EQ(2, testQueue.top());
  testQueue.pop();
  ASSERT_EQ(1, testQueue.top());

  testQueue.push(10);
  ASSERT_EQ(10, testQueue.top());
  testQueue.push(3);
  ASSERT_EQ(10, testQueue.top());
}

TEST_F(XYPriorityQueueTest, QueueOfStruct) {
  struct element_t {
    int value;
    float prio;
    bool operator==(const element_t &rhv) { return value == rhv.value; }
  };

  XYPriorityQueue<element_t> testQueue([](
      const element_t &n1, const element_t &n2) { return n1.prio < n2.prio; });

  testQueue.push({2, 2.0});
  testQueue.push({3, 3.0});
  testQueue.push({1, 1.0});

  ASSERT_EQ(3, testQueue.size());
  ASSERT_EQ(3, testQueue.top().value);
  testQueue.pop();
  ASSERT_EQ(2, testQueue.size());
  ASSERT_EQ(2, testQueue.top().value);
  testQueue.pop();
  ASSERT_EQ(1, testQueue.top().value);
  ASSERT_EQ(1, testQueue.size());

  testQueue.push({10, 10.0});
  ASSERT_EQ(10, testQueue.top().value);
  ASSERT_EQ(2, testQueue.size());

  testQueue.push({3, 3.0});
  ASSERT_EQ(10, testQueue.top().value);
  ASSERT_EQ(3, testQueue.size());

  testQueue.update({3, 20.0});
  ASSERT_EQ(3, testQueue.top().value);
  ASSERT_EQ(3, testQueue.size());

  testQueue.update({10, 21.0});
  ASSERT_EQ(10, testQueue.top().value);
  ASSERT_EQ(3, testQueue.size());
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
