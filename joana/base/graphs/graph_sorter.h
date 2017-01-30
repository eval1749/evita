// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_GRAPHS_GRAPH_SORTER_H_
#define JOANA_BASE_GRAPHS_GRAPH_SORTER_H_

#include <unordered_set>

#include "joana/base/graphs/flow_graph.h"
#include "joana/base/graphs/graph.h"
#include "joana/base/ordered_list.h"

namespace joana {

//////////////////////////////////////////////////////////////////////
//
// Sort graph.
//
template <typename Graph, typename Direction = ForwardFlowGraph<Graph>>
class GraphSorter final {
 public:
  typedef typename Graph::GraphNode GraphNode;
  typedef OrderedList<GraphNode*> OrderedList;

  static OrderedList SortByPreOrder(const Graph* graph);
  static OrderedList SortByPostOrder(const Graph* graph);
  static OrderedList SortByReversePreOrder(const Graph* graph);
  static OrderedList SortByReversePostOrder(const Graph* graph);

 private:
  enum class Reverse {
    No,
    Yes,
  };

  enum class Order {
    PreOrder,
    PostOrder,
  };

  GraphSorter(const Graph* graph, Order order, Reverse reverse);
  ~GraphSorter() = default;

  OrderedList Sort();
  void Visit(typename OrderedList::Builder* builder, GraphNode* node);

  const Graph* const graph_;
  Order const order_;
  Reverse const reverse_;
  std::unordered_set<GraphNode*> visited_;

  DISALLOW_COPY_AND_ASSIGN(GraphSorter);
};

// GraphSorter

// Graph::GraphSorter
template <typename Graph, typename Direction>
GraphSorter<Graph, Direction>::GraphSorter(const Graph* graph,
                                           Order order,
                                           Reverse reverse)
    : graph_(graph), order_(order), reverse_(reverse) {}

template <typename Graph, typename Direction>
OrderedList<typename Graph::GraphNode*> GraphSorter<Graph, Direction>::Sort() {
  OrderedList::Builder builder;
  Visit(&builder, Direction::EntryOf(graph_));
  if (reverse_ == Reverse::Yes)
    builder.Reverse();
  return builder.Get();
}

template <typename Graph, typename Direction>
void GraphSorter<Graph, Direction>::Visit(
    typename OrderedList::Builder* builder,
    GraphNode* node) {
  if (visited_.count(node))
    return;
  visited_.insert(node);
  if (order_ == Order::PreOrder)
    builder->Add(node);
  for (auto const successor : Direction::SuccessorsOf(node))
    Visit(builder, successor);
  if (order_ == Order::PostOrder)
    builder->Add(node);
}

template <typename Graph, typename Direction>
OrderedList<typename Graph::GraphNode*>
GraphSorter<Graph, Direction>::SortByPreOrder(const Graph* graph) {
  return GraphSorter(graph, Order::PreOrder, Reverse::No).Sort();
}

template <typename Graph, typename Direction>
OrderedList<typename Graph::GraphNode*>
GraphSorter<Graph, Direction>::SortByPostOrder(const Graph* graph) {
  return GraphSorter(graph, Order::PostOrder, Reverse::No).Sort();
}

template <typename Graph, typename Direction>
OrderedList<typename Graph::GraphNode*>
GraphSorter<Graph, Direction>::SortByReversePreOrder(const Graph* graph) {
  return GraphSorter(graph, Order::PreOrder, Reverse::Yes).Sort();
}

template <typename Graph, typename Direction>
OrderedList<typename Graph::GraphNode*>
GraphSorter<Graph, Direction>::SortByReversePostOrder(const Graph* graph) {
  return GraphSorter(graph, Order::PostOrder, Reverse::Yes).Sort();
}

}  // namespace joana

#endif  // JOANA_BASE_GRAPHS_GRAPH_SORTER_H_
