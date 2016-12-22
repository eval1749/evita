// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_IR_OPERATOR_H_
#define JOANA_IR_OPERATOR_H_

#include <functional>
#include <tuple>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "joana/base/castable.h"
#include "joana/base/float_type.h"
#include "joana/base/hash_utils.h"
#include "joana/base/memory/zone_allocated.h"
#include "joana/ir/ir_export.h"
#include "joana/ir/operator_forward.h"

namespace joana {
namespace ir {

//
// OperationCode
//
enum class OperationCode {
#define V(name) name,
  FOR_EACH_IR_OPERATOR(V)
#undef V
      NumberOfOperations,
};

const size_t kNumberOfOperations =
    static_cast<size_t>(OperationCode::NumberOfOperations);

//
// Help macros for |Operator| class declaration
//
#define DECLARE_IR_OPERATOR(name, base) DECLARE_CASTABLE_CLASS(name, base);

#define DECLARE_ABSTRACT_IR_OPERATOR(name, base) \
  DECLARE_IR_OPERATOR(name, base);

#define DECLARE_CONCRETE_IR_OPERATOR(name, base) \
  DECLARE_IR_OPERATOR(name, base);               \
  friend class OperatorFactory;

//
// Operator
//
class JOANA_IR_EXPORT Operator : public Castable<Operator>,
                                 public ZoneAllocated {
  DECLARE_ABSTRACT_IR_OPERATOR(Operator, Castable);

 public:
  // Format
  class Format final {
   public:
    class Builder;

    Format();
    ~Format();

    bool is_variadic() const { return is_variadic_; }
    size_t number_of_inputs() const { return number_of_inputs_; }
    size_t number_of_members() const { return number_of_members_; }

   private:
    bool is_variadic_ = false;
    size_t number_of_inputs_ = 0;
    size_t number_of_members_ = 0;
  };

  ~Operator();

  const Format& format() const { return format_; }

  // Shortcuts of format
  bool is_variadic_operator() const { return format_.is_variadic(); }
  size_t number_of_inputs() const { return format_.number_of_inputs(); }

  base::StringPiece mnemonic() const;

  OperationCode opcode() const { return opcode_; }

  // Returns hash code considering operation code and operator members. This
  // function is used for speed up for value numbering optimization.
  virtual size_t ComputeHashCode() const;

  bool operator==(const Operator& other) const;
  bool operator!=(const Operator& other) const;

 protected:
  Operator(OperationCode opcode, const Format& format);

 private:
  const Format format_;
  const OperationCode opcode_;

  DISALLOW_COPY_AND_ASSIGN(Operator);
};

// Operator::Format::Builder
class Operator::Format::Builder final {
 public:
  Builder();
  ~Builder();

  Format Build();

  Builder& set_is_variadic(bool value);
  Builder& set_number_of_inputs(size_t value);
  Builder& set_number_of_members(size_t value);

 private:
  Format format_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

//
// OperatorTemplate
//
template <typename Base, typename... Members>
class OperatorTemplate : public Base {
 protected:
  template <typename... Parameters>
  explicit OperatorTemplate(const std::tuple<Members...>& members,
                            Parameters... parameters)
      : Base(parameters...), members_(members) {}

  ~OperatorTemplate() override = default;

 protected:
  template <size_t kIndex>
  auto member_at() const {
    return std::get<kIndex>(members_);
  }

 private:
  // Implement |Operator| member functions
  size_t ComputeHashCode() const {
    return HashCombine(std::hash<OperationCode>{}(opcode()),
                       std::hash<std::tuple<Members...>>{}(members_));
  }

  std::tuple<Members...> members_;

  DISALLOW_COPY_AND_ASSIGN(OperatorTemplate);
};

#define DECLARE_IR_OPERATOR_0(name)                         \
  class JOANA_IR_EXPORT name##Operator final                \
      : public OperatorTemplate<Operator> {                 \
    DECLARE_CONCRETE_IR_OPERATOR(name##Operator, Operator); \
                                                            \
   public:                                                  \
    ~name##Operator();                                      \
                                                            \
   private:                                                 \
    name##Operator();                                       \
                                                            \
    DISALLOW_COPY_AND_ASSIGN(name##Operator);               \
  };

#define DECLARE_IR_OPERATOR_1(name, type1, member1)         \
  class JOANA_IR_EXPORT name##Operator final                \
      : public OperatorTemplate<Operator, type1> {          \
    DECLARE_CONCRETE_IR_OPERATOR(name##Operator, Operator); \
                                                            \
   public:                                                  \
    ~name##Operator();                                      \
                                                            \
    type1 member1() const { return member_at<0>(); }        \
                                                            \
   private:                                                 \
    name##Operator(type1 member1);                          \
                                                            \
    DISALLOW_COPY_AND_ASSIGN(name##Operator);               \
  };

}  // namespace ir
}  // namespace joana

#endif  // JOANA_IR_OPERATOR_H_
