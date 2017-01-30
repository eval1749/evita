// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_GRAPHS_FLOW_GRAPH_H_
#define JOANA_BASE_GRAPHS_FLOW_GRAPH_H_

namespace joana {

// BackwardFlowGraph is used for computing post-dominator tree
template <typename Graph>
struct BackwardFlowGraph {
  typedef typename Graph::GraphNode GraphNode;

  static GraphNode* EntryOf(const Graph* graph) { return graph->last_node(); }

  static bool HasMoreThanOnePredecessor(const GraphNode* node) {
    return node->HasMoreThanOneSuccessor();
  }

  static bool HasMoreThanOneSuccessor(const GraphNode* node) {
    return node->HasMoreThanOnePredecessor();
  }

  static typename Graph::NodeList PredecessorsOf(const GraphNode* node) {
    return node->successors();
  }

  static typename Graph::NodeList SuccessorsOf(const GraphNode* node) {
    return node->predecessors();
  }
};

// ForwardFlowGraph is used for computing dominator tree
template <typename Graph>
struct ForwardFlowGraph {
  typedef typename Graph::GraphNode GraphNode;

  static GraphNode* EntryOf(const Graph* graph) { return graph->first_node(); }

  static bool HasMoreThanOnePredecessor(const GraphNode* node) {
    return node->HasMoreThanOnePredecessor();
  }

  static bool HasMoreThanOneSuccessor(const GraphNode* node) {
    return node->HasMoreThanOneSuccessor();
  }

  static typename Graph::NodeList PredecessorsOf(const GraphNode* node) {
    return node->predecessors();
  }

  static typename Graph::NodeList SuccessorsOf(const GraphNode* node) {
    return node->successors();
  }
};

}  // namespace joana

#endif  // JOANA_BASE_GRAPHS_FLOW_GRAPH_H_
