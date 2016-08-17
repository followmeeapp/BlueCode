//
//  xy_graph_edge_list.h
//  xy_lmdb_graph
//
//  Created by Erik van der Tier on 13/03/15.
//
//

#ifndef xy_lmdb_graph_xy_graph_edge_list_h
#define xy_lmdb_graph_xy_graph_edge_list_h

#include "xy_lmdb_graph.h"

template <typename IDT>
struct edge_list {
  typedef edge_list<IDT> iterator;
  typedef edge_list<IDT> const_iterator;
  // Iterator methods
  iterator begin();
  iterator end();
  iterator begin() const;
  iterator end() const;

  typedef IDT* value_type;
  typedef size_t size_type;
  typedef IDT*& reference;
  typedef const IDT*& const_reference;
  typedef ptrdiff_t difference_type;

  edge_list(IDT* basePtr, uint64_t size);
  IDT* operator[](const size_type& x);
  bool empty() const;
  size_type size() const;
  size_type max_size() const;

 private:
  IDT* BasePtr_;
  uint64_t Size_;
};

template <typename IDT>
edge_list<IDT>::edge_list(IDT* basePtr, uint64_t size) {}

#endif
