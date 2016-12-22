// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ir/node_inputs.h"

#include "base/logging.h"
#include "joana/ir/node.h"

namespace joana {
namespace ir {

//
// NodeInputs
//
NodeInputs::NodeInputs() {
  edges_.inline_[0].from_ = nullptr;
  edges_.inline_[0].to_ = nullptr;
  edges_.inline_[0].next_ = nullptr;
  edges_.inline_[0].previous_ = nullptr;
}

NodeInputs::~NodeInputs() {
  NOTREACHED();
}

const Node& NodeInputs::input_at(const Node& owner, size_t index) const {
  auto* const edge = UseEdgeAt(owner, index);
  DCHECK_EQ(owner, edge->from_);
  return *edge->to_;
}

size_t NodeInputs::number_of_inputs(const Node& owner) const {
  if (owner.format().is_variadic())
    return edges_.outline_.size_;
  return owner.format().number_of_inputs();
}

void NodeInputs::Init(Zone* zone,
                      const Node& owner,
                      const std::vector<const Node*>& nodes) {
  DCHECK_LE(sizeof(UseEdgeList), sizeof(UseEdge));
  edges_.outline_.capacity_ = nodes.size();
  edges_.outline_.size_ = nodes.size();
  edges_.outline_.elements_ =
      static_cast<UseEdge*>(zone->Allocate(sizeof(UseEdge) * nodes.size()));
  InitUseEdges(edges_.outline_.elements_, owner, nodes);
}

void NodeInputs::Init(const Node& owner,
                      const std::vector<const Node*>& nodes) {
  DCHECK(!owner.format().is_variadic());
  DCHECK_EQ(owner.format().number_of_inputs(), nodes.size());
  InitUseEdges(&edges_.inline_[0], owner, nodes);
}

void NodeInputs::InitAt(size_t index, const Node& from, const Node& to) {
  InitUseEdge(UseEdgeAt(from, index), from, to);
}

void NodeInputs::InitUseEdge(UseEdge* use_edge,
                             const Node& from,
                             const Node& to) {
  auto* const last_first_use_edge = to.inputs_.first_use_edge_;
  use_edge->from_ = &from;
  use_edge->to_ = &to;
  use_edge->next_ = last_first_use_edge;
  use_edge->previous_ = nullptr;
  if (last_first_use_edge) {
    DCHECK(!last_first_use_edge->previous_);
    last_first_use_edge->previous_ = use_edge;
  }
  const_cast<Node&>(to).inputs_.first_use_edge_ = use_edge;
}

void NodeInputs::InitUseEdges(UseEdge* use_edge,
                              const Node& from,
                              const std::vector<const Node*>& to_nodes) {
  auto* runner = use_edge;
  for (const auto& to : to_nodes) {
    InitUseEdge(runner, from, *to);
    ++runner;
  }
}

const NodeInputs::UseEdge* NodeInputs::UseEdgeAt(const Node& owner,
                                                 size_t index) const {
  return const_cast<NodeInputs*>(this)->UseEdgeAt(owner, index);
}

NodeInputs::UseEdge* NodeInputs::UseEdgeAt(const Node& owner, size_t index) {
  if (!owner.format().is_variadic())
    return &edges_.inline_[index];
  DCHECK_LT(index, edges_.outline_.size_);
  return &edges_.outline_.elements_[index];
}

}  // namespace ir
}  // namespace joana
