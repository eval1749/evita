// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_SYNTAX_H_
#define JOANA_AST_SYNTAX_H_

#include <iosfwd>
#include <tuple>

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "joana/ast/ast_export.h"
#include "joana/ast/syntax_forward.h"
#include "joana/base/castable.h"
#include "joana/base/memory/zone_allocated.h"

namespace joana {
namespace ast {

class Node;
class SyntaxVisitor;

//
// SyntaxCode
//
enum class SyntaxCode {
  None,
#define V(name) name,
  FOR_EACH_AST_SYNTAX(V)
#undef V
      NumberOfOperations,
};

const size_t kNumberOfOperations =
    static_cast<size_t>(SyntaxCode::NumberOfOperations);

// Syntax flags
//
#define FOR_EACH_AST_SYNTAX_FLAG_BIT(V) \
  V(Literal, literal)                   \
  V(Variadic, variadic)

//
// Help macros for |Syntax| class declaration
//
#define DECLARE_AST_SYNTAX(name, base) DECLARE_CASTABLE_CLASS(name, base);

#define DECLARE_ABSTRACT_AST_SYNTAX(name, base) DECLARE_AST_SYNTAX(name, base);

#define DECLARE_CONCRETE_AST_SYNTAX(name, base)         \
  DECLARE_AST_SYNTAX(name, base);                       \
  friend class SyntaxFactory;                           \
  static constexpr auto kSyntaxCode = SyntaxCode::name; \
  void Accept(SyntaxVisitor* visitor, const ast::Node& node) const final;

//
// Syntax
//
class JOANA_AST_EXPORT Syntax : public Castable<Syntax>, public ZoneAllocated {
  DECLARE_ABSTRACT_AST_SYNTAX(Syntax, Castable);

 public:
  // Format
  class Format final {
   public:
    class Builder;

    Format();
    ~Format();

    size_t arity() const { return arity_; }
    size_t number_of_parameters() const { return number_of_parameters_; }

#define V(capital, underscore, ...) bool is_##underscore() const;
    FOR_EACH_AST_SYNTAX_FLAG_BIT(V)
#undef V

   private:
    size_t arity_ = 0;
    uint32_t flags_ = 0;
    size_t number_of_parameters_ = 0;
  };

  ~Syntax() override;

  bool operator==(const Syntax& other) const;
  bool operator!=(const Syntax& other) const;
  bool operator==(TokenKind syntax_code) const;
  bool operator!=(TokenKind syntax_code) const;
  bool operator==(SyntaxCode syntax_code) const;
  bool operator!=(SyntaxCode syntax_code) const;

  size_t arity() const { return format_.arity(); }
  const Format& format() const { return format_; }
  base::StringPiece mnemonic() const;
  SyntaxCode opcode() const { return opcode_; }

#define V(capital, underscore, ...) \
  bool is_##underscore() const { return format_.is_##underscore(); }
  FOR_EACH_AST_SYNTAX_FLAG_BIT(V)
#undef V

  virtual void Accept(SyntaxVisitor* visitor, const ast::Node& node) const = 0;

 protected:
  Syntax(SyntaxCode opcode, const Format& format);

 private:
  const Format format_;
  const SyntaxCode opcode_;

  DISALLOW_COPY_AND_ASSIGN(Syntax);
};

// Syntax::Format::Builder
class Syntax::Format::Builder final {
 public:
  Builder();
  ~Builder();

  Format Build();

  Builder& set_arity(size_t value);
  Builder& set_number_of_parameters(size_t value);

#define V(capital, underscore, ...) Builder& set_is_##underscore(bool value);
  FOR_EACH_AST_SYNTAX_FLAG_BIT(V)
#undef V

 private:
  Format format_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

//
// SyntaxTemplate
//
template <typename Base, typename... Parameters>
class SyntaxTemplate : public Base {
 protected:
  template <typename... BaseParameters>
  explicit SyntaxTemplate(const std::tuple<Parameters...>& parameters,
                          BaseParameters... base_parameters)
      : Base(base_parameters...), parameters_(parameters) {}

  ~SyntaxTemplate() override = default;

 protected:
  template <size_t kIndex>
  auto parameter_at() const {
    return std::get<kIndex>(parameters_);
  }

 private:
  std::tuple<Parameters...> parameters_;

  DISALLOW_COPY_AND_ASSIGN(SyntaxTemplate);
};

#define DECLARE_AST_SYNTAX_0(name)                                    \
  class JOANA_AST_EXPORT name final : public SyntaxTemplate<Syntax> { \
    DECLARE_CONCRETE_AST_SYNTAX(name, Syntax);                        \
                                                                      \
   public:                                                            \
    ~name();                                                          \
                                                                      \
   private:                                                           \
    name();                                                           \
                                                                      \
    DISALLOW_COPY_AND_ASSIGN(name);                                   \
  };

#define DECLARE_AST_SYNTAX_1(name, type1, parameter1)                        \
  class JOANA_AST_EXPORT name final : public SyntaxTemplate<Syntax, type1> { \
    DECLARE_CONCRETE_AST_SYNTAX(name, Syntax);                               \
                                                                             \
   public:                                                                   \
    ~name();                                                                 \
                                                                             \
    type1 parameter1() const { return parameter_at<0>(); }                   \
                                                                             \
   private:                                                                  \
    explicit name(type1 parameter1);                                         \
                                                                             \
    DISALLOW_COPY_AND_ASSIGN(name);                                          \
  };

#define DECLARE_AST_SYNTAX_2(name, type1, parameter1, type2, member2) \
  class JOANA_AST_EXPORT name final                                   \
      : public SyntaxTemplate<Syntax, type1, type2> {                 \
    DECLARE_CONCRETE_AST_SYNTAX(name, Syntax);                        \
                                                                      \
   public:                                                            \
    ~name();                                                          \
                                                                      \
    type1 parameter1() const { return parameter_at<0>(); }            \
    type2 parameter2() const { return parameter_at<1>(); }            \
                                                                      \
   private:                                                           \
    name(type1 parameter1, type2 parameter2);                         \
                                                                      \
    DISALLOW_COPY_AND_ASSIGN(name);                                   \
  };

#define IMPLEMENT_AST_SYNTAX_0(base, name, arity)                     \
  name::name()                                                        \
      : SyntaxTemplate(std::tuple<>(), SyntaxCode::name,              \
                       Format::Builder().set_arity(arity).Build()) {} \
  name::~name() = default;

#define IMPLEMENT_AST_SYNTAX_1(base, name, arity, type1, parameter1)  \
  name::name(type1 parameter1)                                        \
      : SyntaxTemplate(std::make_tuple(parameter1), SyntaxCode::name, \
                       Format::Builder()                              \
                           .set_arity(arity)                          \
                           .set_number_of_parameters(1)               \
                           .Build()) {}                               \
  name::~name() = default;

#define IMPLEMENT_AST_SYNTAX_2(base, name, arity, type1, parameter1, type2, \
                               parameter2)                                  \
  name::name(type1 parameter1, type2 parameter2)                            \
      : SyntaxTemplate(std::make_tuple(parameter1, parameter2),             \
                       SyntaxCode::name, Format::Builder()                  \
                                             .set_arity(arity)              \
                                             .set_number_of_parameters(2)   \
                                             .Build()) {}                   \
  name::~name() = default;

// See "syntax_printer.cc" for implementation
JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Syntax& syntax);
JOANA_AST_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Syntax* syntax);

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_SYNTAX_H_
