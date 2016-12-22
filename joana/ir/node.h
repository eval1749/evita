// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_IR_NODE_H_
#define JOANA_IR_NODE_H_

#include <stdint.h>

#include <iterator>
#include <vector>

#include "base/macros.h"
#include "joana/base/double_linked.h"
#include "joana/base/memory/zone_allocated.h"
#include "joana/base/memory/zone_vector.h"
#include "joana/ir/ir_export.h"
#include "joana/ir/node_forward.h"
#include "joana/ir/node_inputs.h"

namespace joana {
namespace ir {

class NodeEditor;

//
// Node
//
class JOANA_IR_EXPORT Node final {
 public:
  // |kMaxNumberOfInputs| should be fit in |Format::number_of_inputs_|.
  static const size_t kMaxNumberOfInputs = 7;

  // Format of this |Node| copied from |Operator::format()| for fast access.
  class JOANA_IR_EXPORT Format final {
   public:
    class Builder;

    Format();
    ~Format();

    bool is_variadic() const { return is_variadic_ != 0; }
    size_t number_of_inputs() const { return number_of_inputs_; }

   private:
    uint32_t is_variadic_ : 1;
    uint32_t number_of_inputs_ : 3;
  };
  static_assert(sizeof(Format) <= sizeof(void*),
                "sizeof(Format) should be small");

  // Inputs
  class JOANA_IR_EXPORT Inputs final {
   public:
    // Iterator is a generator of input node iterator.
    class JOANA_IR_EXPORT Iterator final
        : public std::iterator<std::input_iterator_tag, const Node> {
     public:
      Iterator(const Iterator& other);
      ~Iterator();

      reference operator*() const;
      Iterator& operator++();

      bool operator==(const Iterator& other) const;
      bool operator!=(const Iterator& other) const;

     private:
      friend class Inputs;

      Iterator(const Node& node, size_t index);

      size_t index_;
      const Node* node_;
    };

    Inputs(const Inputs& other);
    ~Inputs();

    Iterator begin() const;
    Iterator end() const;

   private:
    friend class Node;

    explicit Inputs(const Node& node);

    const Node* node_;
  };

  // Users
  class JOANA_IR_EXPORT Users final {
   public:
    // Iterator is a generator of input node iterator.
    class JOANA_IR_EXPORT Iterator final
        : public std::iterator<std::input_iterator_tag, const Node> {
     public:
      explicit Iterator(const iterator& other);
      ~Iterator();

      reference operator*() const;
      Iterator& operator++();

      bool operator==(const Iterator& other) const;
      bool operator!=(const Iterator& other) const;

     private:
      friend class Users;

      Iterator(const Users& owner, Node* node);

      size_t index_;
      const Node* owner_;
    };

    Users(const Users& other);
    ~Users();

    Iterator begin() const;
    Iterator end() const;

   private:
    friend class Node;

    explicit Users(Node* node);

    const Node* node_;
  };

  ~Node();

  // Since |Node| is allocated in |Zone| followed by |UserEdge|, we can't call
  // |delete| and don't need to call.
  void operator delete(void* pointer) = delete;

  bool operator==(const Node& other) const;
  bool operator!=(const Node& other) const;
  bool operator==(const Node* other) const;
  bool operator!=(const Node* other) const;

  const Format& format() const { return format_; }

  // Unique identifier of this |Node|, which is used for index to vector of
  // out-of-line data.
  int id() const { return id_; }

  Inputs inputs() const;
  const Node& input_at(size_t index) const;
  size_t number_of_inputs() const;

  // An output type of this node.
  const Type& output_type() const { return output_type_; }

  // An operator of this node.
  const Operator& op() const { return op_; }

  // A generator of use edge iterator.
  // Users users() const;

  // Type predicates shortcut
  bool is_control() const;
  bool is_effect() const;
  bool is_tuple() const;

 private:
  friend class NodeFactory;
  friend class NodeInputs;

  void* operator new(size_t size, Zone* zone, size_t number_of_inputs);

  // Constructor for variadic nodes, such as Phi.
  Node(Zone* zone,
       const Format& format,
       int id,
       const Operator& op,
       const Type& output_type,
       const std::vector<const Node*>& inputs);

  // Constructor for fixed number of input nodes.
  Node(const Format& format,
       int id,
       const Operator& op,
       const Type& output_type,
       const std::vector<const Node*>& inputs);

  Node(const Format& format,
       int id,
       const Operator& op,
       const Type& output_type,
       const Node& input0,
       const Node& input1);

  Node(const Format& format,
       int id,
       const Operator& op,
       const Type& output_type,
       const Node& input);

  Node(const Format& format,
       int id,
       const Operator& op,
       const Type& output_type);

  Format format_;
  const int id_;
  const Operator& op_;
  const Type& output_type_;

  NodeInputs inputs_;

  DISALLOW_COPY_AND_ASSIGN(Node);
};

}  // namespace ir
}  // namespace joana

#endif  // JOANA_IR_NODE_H_
