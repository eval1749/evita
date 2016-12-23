// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tuple>
#include <unordered_map>

#include "joana/ir/node_factory.h"

#include "joana/base/hash_utils.h"
#include "joana/ir/common_operators.h"
#include "joana/ir/composite_types.h"
#include "joana/ir/node.h"
#include "joana/ir/operator.h"
#include "joana/ir/operator_factory.h"
#include "joana/ir/primitive_types.h"
#include "joana/ir/type_factory.h"

namespace joana {
namespace ir {

//
// Node::Format::Builder
//
class Node::Format::Builder final {
 public:
  explicit Builder(const Operator& op) {
    format_.is_variadic_ = op.is_variadic();
    format_.number_of_inputs_ = op.arity();
  }

  ~Builder() = default;

  Format Build() {
    if (format_.is_variadic())
      DCHECK_EQ(format_.number_of_inputs(), 0);
    return format_;
  }

  Builder& set_is_variadic(bool value) {
    format_.is_variadic_ = value;
    return *this;
  }

  Builder& set_arity(size_t value) {
    DCHECK_LE(value, kMaxNumberOfInputs);
    format_.number_of_inputs_ = value;
    return *this;
  }

 private:
  Node::Format format_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

//
// NodeFactory::Cache
//
class NodeFactory::Cache final {
 public:
  Cache() = default;
  ~Cache() = default;

  // TODO(eval1749): We should share |Cache| implementation with
  // |TypeFactory::Cache| via template.
  template <typename Key>
  const Node* Find(const Key& key) {
    const auto& map = MapFor(key);
    const auto& it = map.find(key);
    return it == map.end() ? nullptr : it->second;
  }

  template <typename Key>
  const Node& Register(const Key& key, const Node& type) {
    auto& map = MapFor(key);
    const auto& result = map.emplace(key, &type);
    DCHECK(result.second);
    return *result.first->second;
  }

 private:
  using Key0 = std::tuple<const Operator*, const Type*>;
  using Key1 = std::tuple<const Operator*, const Type*, const Node*>;
  using Key2 =
      std::tuple<const Operator*, const Type*, const Node*, const Node*>;

  std::unordered_map<Key0, const Node*>& MapFor(const Key0&) { return map_0_; }
  std::unordered_map<Key1, const Node*>& MapFor(const Key1&) { return map_1_; }
  std::unordered_map<Key2, const Node*>& MapFor(const Key2&) { return map_2_; }

  std::unordered_map<Key0, const Node*> map_0_;
  std::unordered_map<Key1, const Node*> map_1_;
  std::unordered_map<Key2, const Node*> map_2_;

  DISALLOW_COPY_AND_ASSIGN(Cache);
};

//
// NodeFactory
//
NodeFactory::NodeFactory(Zone* zone,
                         OperatorFactory* operator_factory,
                         TypeFactory* type_factory)
    : cache_(new Cache()),
      operator_factory_(*operator_factory),
      type_factory_(*type_factory),
      zone_(*zone) {}

NodeFactory::~NodeFactory() = default;

const Node& NodeFactory::GetOrNewNode0(const Operator& op,
                                       const Type& output_type) {
  const auto& key = std::make_tuple(&op, &output_type);
  if (auto* present = cache_->Find(key))
    return *present;
  return cache_->Register(key, NewNode0(op, output_type));
}

const Node& NodeFactory::GetOrNewNode1(const Operator& op,
                                       const Type& output_type,
                                       const Node& input) {
  const auto& key = std::make_tuple(&op, &output_type, &input);
  if (auto* present = cache_->Find(key))
    return *present;
  return cache_->Register(key, NewNode1(op, output_type, input));
}

const Node& NodeFactory::GetOrNewNode2(const Operator& op,
                                       const Type& output_type,
                                       const Node& input0,
                                       const Node& input1) {
  const auto& key = std::make_tuple(&op, &output_type, &input0, &input1);
  if (auto* present = cache_->Find(key))
    return *present;
  return cache_->Register(key, NewNode2(op, output_type, input0, input1));
}

const Node& NodeFactory::GetOrNewNodeN(const Operator& op,
                                       const Type& output_type,
                                       const std::vector<const Node*>& inputs) {
  switch (inputs.size()) {
    case 0:
      return GetOrNewNode0(op, output_type);
    case 1:
      return GetOrNewNode1(op, output_type, *inputs[0]);
    case 2:
      return GetOrNewNode2(op, output_type, *inputs[0], *inputs[1]);
  }
  return NewNodeN(op, output_type, inputs);
}

const Node& NodeFactory::NewNode0(const Operator& op, const Type& output_type) {
  DCHECK(!op.is_variadic()) << op;
  DCHECK_EQ(op.arity(), 0u) << op;
  return *new (&zone_, 0)
      Node(Node::Format::Builder(op).Build(), NextNodeId(), op, output_type);
}

const Node& NodeFactory::NewNode1(const Operator& op,
                                  const Type& output_type,
                                  const Node& input) {
  DCHECK(!op.is_variadic()) << op;
  DCHECK_EQ(op.arity(), 1u) << op;
  return *new (&zone_, 1) Node(Node::Format::Builder(op).Build(), NextNodeId(),
                               op, output_type, input);
}

const Node& NodeFactory::NewNode2(const Operator& op,
                                  const Type& output_type,
                                  const Node& input0,
                                  const Node& input1) {
  DCHECK(!op.is_variadic()) << op;
  DCHECK_EQ(op.arity(), 2u) << op;
  return *new (&zone_, 2) Node(Node::Format::Builder(op).Build(), NextNodeId(),
                               op, output_type, input0, input1);
}

const Node& NodeFactory::NewNodeN(const Operator& op,
                                  const Type& output_type,
                                  const std::vector<const Node*>& inputs) {
  DCHECK(!op.is_variadic()) << op;
  DCHECK_EQ(op.arity(), inputs.size());
  return *new (&zone_, inputs.size()) Node(
      Node::Format::Builder(op).Build(), NextNodeId(), op, output_type, inputs);
}

const Node& NodeFactory::NewVariadicNode(
    const Operator& op,
    const Type& output_type,
    const std::vector<const Node*>& inputs) {
  DCHECK(op.is_variadic()) << op;
  return *new (&zone_, 0) Node(&zone_, Node::Format::Builder(op).Build(),
                               NextNodeId(), op, output_type, inputs);
}

//
// Public node factory
//
const Node& NodeFactory::NewExitNode(const Node& control) {
  DCHECK(control.is_control()) << control;
  return NewNode1(operator_factory_.NewExit(), type_factory_.void_type(),
                  control);
}

const Node& NodeFactory::NewIfNode(const Node& control, const Node& condition) {
  DCHECK(control.is_control()) << control;
  return NewNode2(operator_factory_.NewIf(), type_factory_.control_type(),
                  control, condition);
}

const Node& NodeFactory::NewIfExceptionNode(const Node& control) {
  DCHECK(control.is_control()) << control;
  return NewNode1(operator_factory_.NewIfException(),
                  type_factory_.control_type(), control);
}

const Node& NodeFactory::NewIfFalseNode(const Node& control) {
  DCHECK(control.is_control()) << control;
  return NewNode1(operator_factory_.NewIfFalse(), type_factory_.control_type(),
                  control);
}

const Node& NodeFactory::NewIfSuccessNode(const Node& control) {
  DCHECK(control.is_control()) << control;
  return NewNode1(operator_factory_.NewIfSuccess(),
                  type_factory_.control_type(), control);
}

const Node& NodeFactory::NewIfTrueNode(const Node& control) {
  DCHECK(control.is_control()) << control;
  return NewNode1(operator_factory_.NewIfTrue(), type_factory_.control_type(),
                  control);
}

const Node& NodeFactory::NewLiteralBool(bool data) {
  auto& op = operator_factory_.NewLiteralBool(data);
  auto& type = type_factory_.bool_type();
  return GetOrNewNode0(op, type);
}

const Node& NodeFactory::NewLiteralFloat64(float64_t data) {
  auto& op = operator_factory_.NewLiteralFloat64(data);
  auto& type = type_factory_.float64_type();
  return GetOrNewNode0(op, type);
}

const Node& NodeFactory::NewLiteralInt64(int64_t data) {
  auto& op = operator_factory_.NewLiteralInt64(data);
  auto& type = type_factory_.int64_type();
  return GetOrNewNode0(op, type);
}

const Node& NodeFactory::NewLiteralString(base::StringPiece16 data) {
  auto& op = operator_factory_.NewLiteralString(data);
  auto& type = type_factory_.string_type();
  return GetOrNewNode0(op, type);
}

const Node& NodeFactory::NewLiteralVoid() {
  auto& op = operator_factory_.NewLiteralVoid();
  auto& type = type_factory_.void_type();
  return GetOrNewNode0(op, type);
}

const Node& NodeFactory::NewProjectionNode(const Node& node, size_t index) {
  DCHECK(node.is_tuple()) << node;
  return GetOrNewNode1(operator_factory_.NewProjection(index),
                       node.output_type().As<TupleType>().get(index), node);
}

const Node& NodeFactory::NewRetNode(const Node& control, const Node& node) {
  DCHECK(control.is_control()) << control;
  return GetOrNewNode2(operator_factory_.NewRet(), type_factory_.control_type(),
                       control, node);
}

const Node& NodeFactory::NewStartNode(const Type& output_type) {
  DCHECK(output_type.Is<TupleType>()) << output_type;
  DCHECK_GE(output_type.As<TupleType>().size(), 2) << output_type;
  DCHECK_LE(output_type.As<TupleType>().size(), 3) << output_type;
  DCHECK(output_type.As<TupleType>().get(0).Is<ControlType>()) << output_type;
  return NewNode0(operator_factory_.NewStart(), output_type);
}

const Node& NodeFactory::NewTupleNodeFromVector(
    const std::vector<const Node*>& nodes) {
  std::vector<const Type*> types(nodes.size());
  types.resize(0);
  for (const auto& node : nodes)
    types.emplace_back(&node->output_type());
  auto& output_type = type_factory_.NewTupleTypeFromVector(types);
  return GetOrNewNodeN(operator_factory_.NewTuple(nodes.size()), output_type,
                       nodes);
}

int NodeFactory::NextNodeId() {
  return ++current_node_id_;
}

}  // namespace ir
}  // namespace joana
