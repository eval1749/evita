// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ir/node.h"

#include "joana/ir/composite_types.h"
#include "joana/ir/operator.h"
#include "joana/ir/primitive_types.h"

namespace joana {
namespace ir {

//
// Node::Format
//
Node::Format::Format() : is_variadic_(0), number_of_inputs_(0) {}
Node::Format::~Format() = default;

//
// Node::Inputs::Iterator
//
Node::Inputs::Iterator::Iterator(const Node& node, size_t index)
    : index_(index), node_(&node) {}

Node::Inputs::Iterator::Iterator(const Iterator& other)
    : index_(other.index_), node_(other.node_) {}

Node::Inputs::Iterator::~Iterator() = default;

const Node& Node::Inputs::Iterator::operator*() const {
  return node_->input_at(index_);
}

Node::Inputs::Iterator& Node::Inputs::Iterator::operator++() {
  DCHECK_LT(index_, node_->number_of_inputs());
  ++index_;
  return *this;
}

bool Node::Inputs::Iterator::operator==(const Iterator& other) const {
  DCHECK_EQ(node_, other.node_);
  return index_ == other.index_;
}

bool Node::Inputs::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

//
// Node::Inputs
//
Node::Inputs::Inputs(const Node& node) : node_(&node) {}
Node::Inputs::Inputs(const Inputs& other) : node_(other.node_) {}
Node::Inputs::~Inputs() = default;

Node::Inputs::Iterator Node::Inputs::begin() const {
  return Iterator(*node_, 0);
}

Node::Inputs::Iterator Node::Inputs::end() const {
  return Iterator(*node_, node_->number_of_inputs());
}

//
// Node
//
Node::Node(Zone* zone,
           const Format& format,
           int id,
           const Operator& op,
           const Type& output_type,
           const std::vector<const Node*>& inputs)
    : format_(format), id_(id), op_(op), output_type_(output_type) {
  DCHECK(op_.is_variadic()) << op;
  inputs_.Init(zone, *this, inputs);
}

Node::Node(const Format& format,
           int id,
           const Operator& op,
           const Type& output_type,
           const std::vector<const Node*>& inputs)
    : format_(format), id_(id), op_(op), output_type_(output_type) {
  DCHECK(!op_.is_variadic()) << op;
  DCHECK_EQ(op_.arity(), inputs.size());
  DCHECK(!format_.is_variadic());
  DCHECK_EQ(format_.number_of_inputs(), inputs.size());
  inputs_.Init(*this, inputs);
}

Node::Node(const Format& format,
           int id,
           const Operator& op,
           const Type& output_type,
           const Node& input0,
           const Node& input1)
    : format_(format), id_(id), op_(op), output_type_(output_type) {
  DCHECK(!op_.is_variadic()) << op;
  DCHECK_EQ(op.arity(), 2u) << op;
  DCHECK_EQ(format_.number_of_inputs(), 2u);
  inputs_.InitAt(0, *this, input0);
  inputs_.InitAt(1, *this, input1);
}

Node::Node(const Format& format,
           int id,
           const Operator& op,
           const Type& output_type,
           const Node& input)
    : format_(format), id_(id), op_(op), output_type_(output_type) {
  DCHECK(!op_.is_variadic()) << op;
  DCHECK_EQ(op.arity(), 1u) << op;
  DCHECK_EQ(format_.number_of_inputs(), 1u);
  inputs_.InitAt(0, *this, input);
}

Node::Node(const Format& format,
           int id,
           const Operator& op,
           const Type& output_type)
    : format_(format), id_(id), op_(op), output_type_(output_type) {
  DCHECK(!op_.is_variadic());
  DCHECK_EQ(op.arity(), 0u) << op;
  DCHECK_EQ(format_.number_of_inputs(), 0u);
}

Node::~Node() {
  NOTREACHED();
}

bool Node::operator==(const Node& other) const {
  return this == &other;
}
bool Node::operator!=(const Node& other) const {
  return !operator==(other);
}
bool Node::operator==(const Node* other) const {
  return this == other;
}
bool Node::operator!=(const Node* other) const {
  return !operator==(other);
}

void* Node::operator new(size_t size, Zone* zone, size_t arity) {
  return zone->Allocate(size + arity * sizeof(NodeInputs::UseEdge));
}

Node::Inputs Node::inputs() const {
  return Inputs(*this);
}

const Node& Node::input_at(size_t index) const {
  DCHECK_LT(index, number_of_inputs());
  return inputs_.input_at(*this, index);
}

bool Node::is_control() const {
  if (output_type().Is<ControlType>())
    return true;
  auto* tuple = output_type().TryAs<TupleType>();
  if (!tuple || tuple->size() == 0)
    return false;
  return tuple->get(0).Is<ControlType>();
}

bool Node::is_effect() const {
  if (output_type().Is<EffectType>())
    return true;
  auto* tuple = output_type().TryAs<TupleType>();
  if (!tuple || tuple->size() < 2)
    return false;
  return tuple->get(1).Is<EffectType>();
}

bool Node::is_tuple() const {
  return output_type().Is<TupleType>();
}

size_t Node::number_of_inputs() const {
  return inputs_.number_of_inputs(*this);
}

}  // namespace ir
}  // namespace joana
