//
//  xy_lmdb_graph_tests.cpp
//  PageRank
//
//  Created by Erik van der Tier on 02/03/15.
//
//

#include <gtest/gtest.h>
#include "discovery_engine/xy_lmdb_graph/xy_lmdb_graph.h"

using namespace XYGraphEngine;
using namespace LMDB;
namespace {

// The fixture for testing class Foo.
class lmdb_graph_test : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  lmdb_graph_test() {
    // You can do set-up work for each test here.
  }

  virtual ~lmdb_graph_test() {
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

TEST_F(lmdb_graph_test, Base) {
  ErrorPtr e;

  std::string test;
  auto Graph = lmdb_graph<std::string, double>(
      e, "/Users/erik/Documents/Suiron/Data/graph_test", 1024);
  std::cout << Graph.getEnv().getVersionString() << std::endl;
  if (e) std::cout << e.get()->message() << std::endl;
  ON_ERROR_RETURN(e);
  ERROR_RESET(e);

  {
    auto txn1 = Graph.beginTransaction(e);
    int64_t id = 1;
    Graph.addVertex(e, id, "test", txn1);
    auto out = Graph.getVertex(e, id, txn1);
    std::cout << *out << std::endl;
  }

  ERROR_RESET(e);
  Graph.addVertex(e, 1, "test");
  ASSERT_FALSE(e);

  ERROR_RESET(e);
  Graph.addVertex(e, 3, "test3");
  ASSERT_FALSE(e);

  ERROR_RESET(e);
  Graph.addVertex(e, 4, "test4");
  ASSERT_FALSE(e);

  ERROR_RESET(e);
  Graph.addVertex(e, 2, "test2");
  ASSERT_FALSE(e);

  auto Vertex1 = Graph.getVertex(e, 1);
  ASSERT_TRUE(Vertex1.is_initialized());
  std::cout << *Vertex1 << std::endl;
  // if (Vertex1) ASSERT_EQ("test", (char*)&*Vertex1);

  ERROR_RESET(e);
  auto Vertex2 = Graph.getVertex(e, 2);
  ASSERT_TRUE(Vertex2.is_initialized());
  std::cout << *Vertex2 << std::endl;
  //  if (Vertex2) ASSERT_EQ("test2", *Vertex2);

  ERROR_RESET(e);
  auto txn = Graph.beginTransaction(e);
  ASSERT_FALSE(e);

  std::map<uint64_t, std::vector<uint64_t>> InEdgesMap;
  std::map<uint64_t, std::vector<uint64_t>> OutEdgesMap;

  ERROR_RESET(e);
  Graph.addEdgeBulk(e, 1, 2, 0.5, txn);
  ASSERT_FALSE(e);

  InEdgesMap[2].push_back(1);
  OutEdgesMap[1].push_back(2);

  ERROR_RESET(e);
  Graph.addEdgeBulk(e, 1, 3, 0.1, txn);
  ASSERT_FALSE(e);

  InEdgesMap[3].push_back(1);
  OutEdgesMap[1].push_back(3);

  ERROR_RESET(e);
  Graph.addEdgeBulk(e, 3, 1, 0.6, txn);
  ASSERT_FALSE(e);

  InEdgesMap[1].push_back(3);
  OutEdgesMap[3].push_back(1);

  ERROR_RESET(e);
  Graph.addEdgeBulk(e, 4, 1, 0.8, txn);
  ASSERT_FALSE(e);

  InEdgesMap[1].push_back(4);
  OutEdgesMap[4].push_back(1);

  ERROR_RESET(e);
  Graph.AjdListUpdate(e, 1, InEdgesMap[1], OutEdgesMap[1], txn);
  ASSERT_FALSE(e);
  ERROR_RESET(e);
  Graph.AjdListUpdate(e, 2, InEdgesMap[2], OutEdgesMap[2], txn);
  ASSERT_FALSE(e);
  ERROR_RESET(e);
  Graph.AjdListUpdate(e, 3, InEdgesMap[3], OutEdgesMap[3], txn);
  ASSERT_FALSE(e);
  ERROR_RESET(e);
  Graph.AjdListUpdate(e, 4, InEdgesMap[4], OutEdgesMap[4], txn);
  ASSERT_FALSE(e);

  ERROR_RESET(e);
  Graph.commitTransaction(e, txn);
  ASSERT_FALSE(e);

  ERROR_RESET(e);
  auto Edge1 = Graph.getEdge(e, 1, 2);
  ASSERT_TRUE(Edge1.is_initialized());
  if (Edge1) ASSERT_EQ(0.5, *Edge1);

  ERROR_RESET(e);
  auto Edge2 = Graph.getEdge(e, 1, 3);
  ASSERT_TRUE(Edge2.is_initialized());
  if (Edge2) ASSERT_EQ(0.1, *Edge2);

  ERROR_RESET(e);
  Graph.addEdge(e, 2, 1, 0.1);

  ERROR_RESET(e);
  auto AnyEdges = Graph.getAnyEdges(e, 1, 2);
  ASSERT_EQ(2, AnyEdges.size());

  ERROR_RESET(e);
  auto OutDegree = Graph.getOutDegree(e, 1);
  ASSERT_EQ(2, OutDegree);

  ERROR_RESET(e);
  auto InDegree = Graph.getInDegree(e, 1);
  ASSERT_EQ(3, InDegree);

  ERROR_RESET(e);
  auto Degree = Graph.getDegree(e, 1);
  ASSERT_EQ(5, Degree);

  ERROR_RESET(e);
  Graph.removeEdge(e, 2, 1);
  ERROR_RESET(e);
  AnyEdges = Graph.getAnyEdges(e, 1, 2);
  ASSERT_EQ(1, AnyEdges.size());
  ASSERT_EQ(4, Graph.getDegree(e, 1));

  ERROR_RESET(e);
  auto txn2 = Graph.beginTransaction(e);
  OutDegree = Graph.getOutDegree(e, 1, txn2);
  ASSERT_EQ(2, OutDegree);
  Graph.commitTransaction(e, txn2);

  ERROR_RESET(e);
  auto Edges = Graph.getEdges(e, 1);

  std::cout << "Edges: \n";
  for (auto &&e : Edges)
    std::cout << e.vertexID1 << ", " << e.vertexID2 << ": " << e.properties
              << "\n";

  ERROR_RESET(e);
  auto InEdges = Graph.getInEdges(e, 1);

  std::cout << "InEdges: \n";
  for (auto &&e : InEdges)
    std::cout << e.vertexID1 << ", " << e.vertexID2 << ": " << e.properties
              << "\n";

  ERROR_RESET(e);
  auto OutEdges = Graph.getOutEdges(e, 1);

  std::cout << "OutEdges: \n";
  for (auto &&e : OutEdges)
    std::cout << e.vertexID1 << ", " << e.vertexID2 << ": " << e.properties
              << "\n";
  ASSERT_FALSE(e);

  ERROR_RESET(e);
  Graph.forEachInEdge(
      e, 1,
      [](ErrorPtr &e, const uint64_t *vertexID1, const uint64_t *vertexID2,
         const DBVal *properties, LMDB::Transaction &txn) {
        std::cout << "VertexID1: " << *vertexID1
                  << "\nVertexID2: " << *vertexID2 << "\n";
      });
  // ASSERT_EQ(MDB_NOTFOUND, e.get()->code);

  ERROR_RESET(e);
  auto txn3 = Graph.beginTransaction(e);
  ASSERT_FALSE(e);
  Graph.forEachInEdge(
      e, 1, txn3,
      [](ErrorPtr &e, const uint64_t *vertexID1, const uint64_t *vertexID2,
         const DBVal *properties, LMDB::Transaction &txn) {
        std::cout << "VertexID1: " << *vertexID1
                  << "\nVertexID2: " << *vertexID2 << "\n";
      });
  // ASSERT_EQ(MDB_NOTFOUND, e.get()->code);

  ERROR_RESET(e);
  Graph.commitTransaction(e, txn3);
  ASSERT_FALSE(e);

  std::cout << "\n Start out Edges!" << std::endl;

  ERROR_RESET(e);
  Graph.forEachOutEdge(
      e, 1,
      [](ErrorPtr &e, const uint64_t *vertexID1, const uint64_t *vertexID2,
         const DBVal *properties, LMDB::Transaction &txn) {
        std::cout << "VertexID1: " << *vertexID1
                  << "\nVertexID2: " << *vertexID2 << "\n";
      });
  // ASSERT_EQ(MDB_NOTFOUND, e.get()->code);

  ERROR_RESET(e);
  auto txn4 = Graph.beginTransaction(e);
  ASSERT_FALSE(e);
  std::cout << "\n Start In with nested out Edges!" << std::endl;
  Graph.forEachInEdge(
      e, 1, txn4, [&Graph](ErrorPtr &e, const uint64_t *vertexID1,
                           const uint64_t *vertexID2, const DBVal *properties,
                           LMDB::Transaction &txn) {
        std::cout << "InVertexID1: " << *vertexID1
                  << "\nInVertexID2: " << *vertexID2
                  << "\nInOutDegree: " << Graph.getOutDegree(e, *vertexID2, txn)
                  << "\n";
        ERROR_RESET(e);
        Graph.forEachOutEdge(
            e, *vertexID1, txn,
            [&Graph](ErrorPtr &e, const uint64_t *vertexID1,
                     const uint64_t *vertexID2, const DBVal *properties,
                     LMDB::Transaction &txn) {
              std::cout << "OutVertexID1: " << *vertexID1
                        << "\nOutVertexID2: " << *vertexID2 << "\nOutInDegree: "
                        << Graph.getInDegree(e, *vertexID2, txn) << "\n";
              ERROR_RESET(e);
            });
      });
  // ASSERT_EQ(MDB_NOTFOUND, e.get()->code);

  ERROR_RESET(e);
  Graph.commitTransaction(e, txn4);
  ASSERT_FALSE(e);

  ERROR_RESET(e);
  Graph.forEachVertex(e, [&Graph](ErrorPtr &e, const uint64_t *vertexID,
                                  const DBVal *properties,
                                  LMDB::Transaction &txn) {
    std::cout << "Vertex: " << *vertexID << "\n"
              << "Value:  " << (char *)properties->buffer() << "\n";
    Graph.forEachInEdge(
        e, *vertexID, txn,
        [](ErrorPtr &e, const uint64_t *vertexID1, const uint64_t *vertexID2,
           const DBVal *properties, LMDB::Transaction &txn) {
          std::cout << "VertexID1: " << *vertexID1
                    << "\nVertexID2: " << *vertexID2 << "\n";
        });
    ERROR_RESET(e);
  });
  ERROR_RESET(e);

  ERROR_RESET(e);
  Graph.removeVertex(e, 1);
  ASSERT_FALSE(e);
  auto RemovedVertex = Graph.getVertex(e, 1);
  ASSERT_FALSE(e);
  ASSERT_FALSE(RemovedVertex.is_initialized());

  Graph.remove(e);
  ASSERT_FALSE(e);
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
