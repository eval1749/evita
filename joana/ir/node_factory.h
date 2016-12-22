// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_IR_NODE_FACTORY_H_
#define JOANA_IR_NODE_FACTORY_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "joana/base/float_type.h"
#include "joana/ir/ir_export.h"
#include "joana/ir/node.h"

namespace joana {

class Zone;

namespace ir {

class Node;
class Operator;
class OperatorFactory;
class Type;
class TypeFactory;

//
// NodeFactory
//
class JOANA_IR_EXPORT NodeFactory final {
 public:
  NodeFactory(Zone* zone,
              OperatorFactory* operator_factory,
              TypeFactory* type_factory);
  ~NodeFactory();

  // Returns new Exit node.
  //  - No output
  //  - One input node of Control.
  const Node& NewExitNode(const Node& control);

  const Node& NewLiteralBool(bool data);
  const Node& NewLiteralFloat64(float64_t data);
  const Node& NewLiteralInt64(int64_t data);
  const Node& NewLiteralString(base::StringPiece16 data);
  const Node& NewLiteralVoid();

  // Returns new Project node.
  //  - |node| is |TupleType|
  //  - output type is a type of |index|th member of |TupleType|
  const Node& NewProjectionNode(const Node& node, size_t index);

  // Returns new Ret node.
  // - |node| is value of function.
  const Node& NewRetNode(const Node& control, const Node& node);

  // Returns new Start node.
  // |output_type| should be either
  //  - Tuple<Control, Effect, Tuple<...>>; has side effect
  //  - Tuple<Control, Tuple<...>>; has no side effect
  // Third member of tuple represents type of function parameters.
  const Node& NewStartNode(const Type& output_type);

  // Returns new Tuple node.
  const Node& NewTupleNodeFromVector(const std::vector<const Node*>& nodes);

  template <typename... Members>
  const Node& NewTupleNode(const Members&... members) {
    return NewTupleNodeFromVector({&members...});
  }

 private:
  class Cache;

  const Node& GetOrNewNode0(const Operator& op, const Type& output_type);

  const Node& GetOrNewNode1(const Operator& op,
                            const Type& output_type,
                            const Node& input);

  const Node& GetOrNewNode2(const Operator& op,
                            const Type& output_type,
                            const Node& input0,
                            const Node& input1);

  const Node& GetOrNewNodeN(const Operator& op,
                            const Type& output_type,
                            const std::vector<const Node*>& inputs);

  const Node& NewNode0(const Operator& op, const Type& output_type);

  const Node& NewNode1(const Operator& op,
                       const Type& output_type,
                       const Node& input);

  const Node& NewNode2(const Operator& op,
                       const Type& output_type,
                       const Node& input0,
                       const Node& input1);

  const Node& NewNodeN(const Operator& op,
                       const Type& output_type,
                       const std::vector<const Node*>& inputs);

  const Node& NewVariadicNode(const Operator& op,
                              const Type& output_type,
                              const std::vector<const Node*>& inputs);

  int NextNodeId();

  const std::unique_ptr<Cache> cache_;
  int current_node_id_ = 0;
  OperatorFactory& operator_factory_;
  TypeFactory& type_factory_;
  Zone& zone_;

  DISALLOW_COPY_AND_ASSIGN(NodeFactory);
};

}  // namespace ir
}  // namespace joana

#endif  // JOANA_IR_NODE_FACTORY_H_
