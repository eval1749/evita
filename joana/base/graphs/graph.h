// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_GRAPHS_GRAPH_H_
#define JOANA_BASE_GRAPHS_GRAPH_H_

#include <algorithm>

#include "base/logging.h"
#include "joana/base/double_linked.h"
#include "joana/base/memory/zone_unordered_set.h"
#include "joana/base/memory/zone_vector.h"
#include "joana/base/ordered_list.h"

namespace joana {

template <typename Graph>
struct ForwardFlowGraph;

template <typename Graph>
class GraphEditor;

template <typename Graph, typename Direction>
class GraphSorter;

//////////////////////////////////////////////////////////////////////
//
// Represents directed graph. It is OK to have cycle.
//
template <typename Owner, typename Node>
class Graph {
 public:
  class Editor;
  class GraphNodeBase;

  using GraphNode = Node;
  using NodeList = ZoneVector<GraphNode*>;
  using NodeSet = ZoneUnorderedSet<GraphNode*>;
  using Nodes = DoubleLinked<GraphNode, Owner>;
  using Sorter = GraphSorter<Graph, ForwardFlowGraph<Graph>>;

  // |NodeBase| represents graph node having edges.
  class GraphNodeBase : public DoubleLinked<GraphNode, Owner>::NodeBase {
   public:
    const NodeList& predecessors() const { return predecessors_; }
    const NodeList& successors() const { return successors_; }

    bool HasMoreThanOnePredecessor() const { return predecessors_.size() > 1u; }
    bool HasMoreThanOneSuccessor() const { return successors_.size() > 1u; }
    bool HasPredecessor() const { return !predecessors_.empty(); }
    bool HasSuccessor() const { return !successors_.empty(); }

   protected:
    explicit GraphNodeBase(Zone* zone);
    ~GraphNodeBase() = default;

   private:
    friend class Editor;
    friend class Graph;

    // For control flow graph, most of basic blocks have few successors, e.g.
    // by unconditional jump, and return. More than two successors are
    // introduced by switch like statement.
    NodeList predecessors_;
    NodeList successors_;

    DISALLOW_COPY_AND_ASSIGN(GraphNodeBase);
  };

  // Returns first block.
  GraphNode* first_node() const { return nodes_.first_node(); }
  GraphNode* last_node() const { return nodes_.last_node(); }

  // Returns a list of graph node.
  const Nodes& nodes() const { return nodes_; }

  bool HasEdge(GraphNode* from, GraphNode* to) const;

 protected:
  Graph() = default;
  ~Graph() = default;

 private:
  friend class Editor;

  static bool Has(const NodeList& nodes, GraphNode* node);

  Nodes nodes_;

  DISALLOW_COPY_AND_ASSIGN(Graph);
};

// Graph
template <typename Owner, typename T>
bool Graph<Owner, T>::Has(const NodeList& nodes, GraphNode* node) {
  auto const it = std::find(nodes.begin(), nodes.end(), node);
  return it != nodes.end();
}

template <typename Owner, typename T>
bool Graph<Owner, T>::HasEdge(GraphNode* from, GraphNode* to) const {
  if (Has(from->successors_, to)) {
    DCHECK(Has(to->predecessors_, from));
    return true;
  }
  DCHECK(!Has(to->predecessors_, from));
  return false;
}

// Graph::GraphNodeBase
template <typename Owner, typename T>
Graph<Owner, T>::GraphNodeBase::GraphNodeBase(Zone* zone)
    : predecessors_(zone), successors_(zone) {}

}  // namespace joana

#endif  // JOANA_BASE_GRAPHS_GRAPH_H_
