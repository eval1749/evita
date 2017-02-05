// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_GRAPHS_GRAPH_EDITOR_H_
#define JOANA_BASE_GRAPHS_GRAPH_EDITOR_H_

#include <algorithm>

#include "base/logging.h"
#include "joana/base/graphs/graph.h"

namespace joana {

//////////////////////////////////////////////////////////////////////
//
// Editor edits a Graph.
//
template <typename G, typename N>
class Graph<G, N>::Editor {
 public:
  using Node = typename Graph::GraphNode;
  using NodeList = typename Graph::NodeList;

  Editor() = default;
  ~Editor() = default;

  Editor& AppendNode(Graph* graph, Node* new_node);
  Editor& AddEdge(Graph* graph, Node* from, Node* to);
  Editor& InsertNode(Graph* graph, Node* new_node, Node* ref_node);
  Editor& RemoveEdge(Graph* graph, Node* from, Node* to);
  Editor& RemoveNode(Graph* graph, Node* ref_node);

 private:
  static void RemoveFromList(NodeList* nodes, Node* node);

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

template <typename G, typename N>
typename Graph<G, N>::Editor& Graph<G, N>::Editor::AppendNode(Graph* graph,
                                                              Node* new_node) {
  graph->nodes_.AppendNode(new_node);
  return *this;
}

template <typename G, typename N>
typename Graph<G, N>::Editor& Graph<G, N>::Editor::AddEdge(Graph* graph,
                                                           Node* from,
                                                           Node* to) {
  DCHECK(!graph->HasEdge(from, to));
  from->successors_.push_back(to);
  to->predecessors_.push_back(from);
  return *this;
}

template <typename G, typename N>
typename Graph<G, N>::Editor& Graph<G, N>::Editor::InsertNode(Graph* graph,
                                                              Node* new_node,
                                                              Node* ref_node) {
  graph->nodes_.InsertBefore(new_node, ref_node);
  return *this;
}

template <typename G, typename N>
typename Graph<G, N>::Editor& Graph<G, N>::Editor::RemoveEdge(Graph* graph,
                                                              Node* from,
                                                              Node* to) {
  DCHECK(graph->HasEdge(from, to));
  RemoveFromList(&from->successors_, to);
  RemoveFromList(&to->predecessors_, from);
  return *this;
}

template <typename G, typename N>
void Graph<G, N>::Editor::RemoveFromList(NodeList* nodes, Node* node) {
  auto const it = std::find(nodes->begin(), nodes->end(), node);
  DCHECK(it != nodes->end());
  nodes->erase(it);
}

template <typename G, typename N>
typename Graph<G, N>::Editor& Graph<G, N>::Editor::RemoveNode(Graph* graph,
                                                              Node* old_node) {
  graph->nodes_.RemoveNode(old_node);
  return *this;
}

}  // namespace joana

#endif  // JOANA_BASE_GRAPHS_GRAPH_EDITOR_H_
