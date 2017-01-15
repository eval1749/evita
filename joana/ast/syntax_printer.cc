// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>

#include "joana/ast/syntax.h"

#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/lexical_grammar.h"
#include "joana/ast/regexp.h"
#include "joana/ast/types.h"

namespace joana {
namespace ast {

namespace {

template <typename T>
struct Printable {
  const T* value;
};

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<AssignmentExpression>& printable) {
  const auto& syntax = *printable.value;
  return ostream << '<' << syntax.op() << '>';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<BinaryExpression>& printable) {
  const auto& syntax = *printable.value;
  return ostream << '<' << syntax.op() << '>';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Function>& printable) {
  const auto& syntax = *printable.value;
  return ostream << '<' << syntax.kind() << '>';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<FunctionType>& printable) {
  const auto& syntax = *printable.value;
  return ostream << '<' << syntax.kind() << '>';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Method>& printable) {
  const auto& syntax = *printable.value;
  return ostream << '<' << syntax.method_kind() << ',' << syntax.kind() << '>';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<RegExpRepeat>& printable) {
  const auto& syntax = *printable.value;
  const auto min = syntax.min();
  const auto max = syntax.max();
  const auto method = syntax.is_lazy() ? "?" : "";
  if (min == 0 && max == 1)
    return ostream << "<?" << method << '>';
  if (min == 0 && max == kRegExpInfinity)
    return ostream << "<*" << method << '>';
  if (min == 1 && max == kRegExpInfinity)
    return ostream << "<+" << method << '>';
  if (min == max)
    return ostream << "<{" << min << '}' << method << '>';
  if (max == kRegExpInfinity)
    return ostream << "<{" << min << ",}" << method << '>';
  return ostream << "<{" << min << ',' << max << '}' << method << '>';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<UnaryExpression>& printable) {
  const auto& syntax = *printable.value;
  return ostream << '<' << syntax.op() << '>';
}

}  // namespace

// FunctionKind
std::ostream& operator<<(std::ostream& ostream, FunctionKind kind) {
  static const char* const kTexts[] = {
#define V(name) #name,
      FOR_EACH_AST_FUNCTION_KIND(V)
#undef V
  };
  const auto& it = std::begin(kTexts) + static_cast<size_t>(kind);
  if (it < std::begin(kTexts) || it >= std::end(kTexts))
    return ostream << "FunctionKind" << static_cast<size_t>(kind);
  return ostream << *it;
}

// FunctionTypeKind
std::ostream& operator<<(std::ostream& ostream, FunctionTypeKind kind) {
  static const char* const kTexts[] = {
#define V(name) #name,
      FOR_EACH_AST_FUNCTION_TYPE_KIND(V)
#undef V
  };
  const auto& it = std::begin(kTexts) + static_cast<size_t>(kind);
  if (it < std::begin(kTexts) || it >= std::end(kTexts))
    return ostream << "FunctionTypeKind" << static_cast<size_t>(kind);
  return ostream << *it;
}

// MethodKind
std::ostream& operator<<(std::ostream& ostream, MethodKind kind) {
  static const char* const kTexts[] = {
#define V(name) #name,
      FOR_EACH_AST_METHOD_KIND(V)
#undef V
  };
  const auto& it = std::begin(kTexts) + static_cast<size_t>(kind);
  if (it < std::begin(kTexts) || it >= std::end(kTexts))
    return ostream << "MethodKind" << static_cast<size_t>(kind);
  return ostream << *it;
}

// TokenKind
std::ostream& operator<<(std::ostream& ostream, TokenKind kind) {
  static const char* const kTexts[] = {
#define V(text, ...) text,
      FOR_EACH_JAVASCRIPT_PUNCTUATOR(V)
#undef V
          "StartOfKeyword",
#define V(name, camel, upper) #name,
      FOR_EACH_JAVASCRIPT_KEYWORD(V)
#undef V
          "EndOfKeyword",

      "StartOfKnownWord",
#define V(name, camel, upper) #name,
      FOR_EACH_JAVASCRIPT_KNOWN_WORD(V)
#undef V
          "EndOfKnownWord",

      "StartOfJsDocTagName,"
#define V(name, camel, syntax) "@" #name,
      FOR_EACH_JSDOC_TAG_NAME(V)
#undef V
          "EndOfJsDocTagName",
  };
  const auto& it = std::begin(kTexts) + static_cast<size_t>(kind);
  if (it < std::begin(kTexts) || it >= std::end(kTexts))
    return ostream << "TokenKind" << static_cast<size_t>(kind);
  return ostream << *it;
}

// Syntax
std::ostream& operator<<(std::ostream& ostream, const Syntax& syntax) {
  ostream << syntax.mnemonic();
  if (syntax.format().number_of_parameters() == 0)
    return ostream;

#define PRINT_PARAMETERS_IF(name) \
  if (syntax.Is<name>())          \
    return ostream << Printable<name>{&syntax.As<name>()};

  PRINT_PARAMETERS_IF(AssignmentExpression);
  PRINT_PARAMETERS_IF(BinaryExpression);
  PRINT_PARAMETERS_IF(Function);
  PRINT_PARAMETERS_IF(FunctionType);
  PRINT_PARAMETERS_IF(Method);
  PRINT_PARAMETERS_IF(RegExpRepeat);
  PRINT_PARAMETERS_IF(UnaryExpression);
#undef PRINT_PARAMETERS

  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Syntax* syntax) {
  if (!syntax)
    return ostream << "(null)";
  return ostream << *syntax;
}

std::ostream& operator<<(std::ostream& ostream, SyntaxCode syntax_code) {
  static const char* const kTexts[] = {
    "None",
#define V(name) #name,
    FOR_EACH_AST_SYNTAX(V)
#undef V
  };
  const auto& it = std::begin(kTexts) + static_cast<size_t>(syntax_code);
  if (it < std::begin(kTexts) || it >= std::end(kTexts))
    return ostream << "SyntaxCode" << static_cast<size_t>(syntax_code);
  return ostream << *it;
}

}  // namespace ast
}  // namespace joana
