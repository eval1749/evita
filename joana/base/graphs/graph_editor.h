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

  explicit Editor(Graph* graph) : graph_(graph) {}
  ~Editor() = default;

  void AppendNode(Node* new_node);
  void AddEdge(Node* from, Node* to);
  void InsertNode(Node* new_node, Node* ref_node);
  void RemoveEdge(Node* from, Node* to);
  void RemoveNode(Node* ref_node);

 private:
  static void RemoveFromList(NodeList* nodes, Node* node);

  Graph* const graph_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

template <typename G, typename N>
void Graph<G, N>::Editor::AppendNode(Node* new_node) {
  graph_->nodes_.AppendNode(new_node);
}

template <typename G, typename N>
void Graph<G, N>::Editor::AddEdge(Node* from, Node* to) {
  DCHECK(!graph_->HasEdge(from, to));
  from->successors_.push_back(to);
  to->predecessors_.push_back(from);
}

template <typename G, typename N>
void Graph<G, N>::Editor::InsertNode(Node* new_node, Node* ref_node) {
  graph_->nodes_.InsertBefore(new_node, ref_node);
}

template <typename G, typename N>
void Graph<G, N>::Editor::RemoveEdge(Node* from, Node* to) {
  RemoveFromList(&from->successors_, to);
  RemoveFromList(&to->predecessors_, from);
}

template <typename G, typename N>
void Graph<G, N>::Editor::RemoveFromList(NodeList* nodes, Node* node) {
  auto const it = std::find(nodes->begin(), nodes->end(), node);
  DCHECK(it != nodes->end());
  nodes->erase(it);
}

template <typename G, typename N>
void Graph<G, N>::Editor::RemoveNode(Node* old_node) {
  graph_->nodes_.RemoveNode(old_node);
}

}  // namespace joana

#endif  // JOANA_BASE_GRAPHS_GRAPH_EDITOR_H_
