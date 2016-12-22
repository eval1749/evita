// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_IR_NODE_INPUTS_H_
#define JOANA_IR_NODE_INPUTS_H_

#include <vector>

#include "base/macros.h"

namespace joana {
class Zone;
namespace ir {

class Node;

//
// NodeInputs represens input nodes of |Node| with use chain.
//
class NodeInputs final {
 public:
  //
  // UseEdge
  //
  // The instances of this class are allocated into |Zone|.
  //
  struct UseEdge final {
    const Node* from_;
    const Node* to_;
    UseEdge* next_;
    UseEdge* previous_;
  };

  NodeInputs();
  ~NodeInputs();

  // Input node getter for Node::Inputs::Iterator
  const Node& input_at(const Node& owner, size_t index) const;
  size_t number_of_inputs(const Node& owner) const;

  void Init(Zone* zone,
            const Node& owner,
            const std::vector<const Node*>& nodes);

  void Init(const Node& owner, const std::vector<const Node*>& nodes);

  // Initialize |use_edge| and link to |to|'s use edge list.
  void InitAt(size_t index, const Node& from, const Node& to);

 private:
  //
  // UseEdgeList
  //
  struct UseEdgeList final {
    size_t capacity_;
    size_t size_;
    UseEdge* elements_;
  };

  static_assert(sizeof(UseEdgeList) <= sizeof(UseEdge),
                "UseEdgeList must be smaller than or equal to UseEdge");

  // Initialize use edge specified by |use_edge| with |to|.
  static void InitUseEdge(UseEdge* use_edge, const Node& from, const Node& to);

  // Initialize use edge starts from |use_edge| with |input_nodes|.
  static void InitUseEdges(UseEdge* use_edge,
                           const Node& from,
                           const std::vector<const Node*>& input_nodes);

  const UseEdge* UseEdgeAt(const Node& owner, size_t index) const;
  UseEdge* UseEdgeAt(const Node& owner, size_t index);

  // A list of used nodes of this node.
  UseEdge* first_use_edge_ = nullptr;

  // |inputs_| must be the last member field. We allocate |UseEdge| after
  // |Node|.
  union {
    UseEdge inline_[1];
    UseEdgeList outline_;
  } edges_;

  DISALLOW_COPY_AND_ASSIGN(NodeInputs);
};

}  // namespace ir
}  // namespace joana

#endif  // JOANA_IR_NODE_INPUTS_H_
