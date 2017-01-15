// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_CONTEXT_H_
#define JOANA_ANALYZER_CONTEXT_H_

#include <memory>
#include <unordered_map>

#include "base/macros.h"

namespace joana {

namespace ast {
class Node;
}

class AnalyzerSettings;
class ErrorSink;
class SourceCodeRange;
class Zone;

namespace analyzer {

enum class ErrorCode;
class Factory;
class Type;
class TypeFactory;
class TypeMap;
class Value;
class ValueMap;

//
// Context
//
class Context final {
 public:
  explicit Context(const AnalyzerSettings& settings);
  ~Context();

  ErrorSink& error_sink() const;
  Factory& factory() const { return *factory_; }
  TypeFactory& type_factory() const { return *type_factory_; }

  void AddError(const ast::Node& node,
                ErrorCode error_code,
                const ast::Node& related);
  void AddError(const ast::Node& node, ErrorCode error_code);
  void AddError(const SourceCodeRange& range, ErrorCode error_code);

  // Query
  const Type* TryTypeOf(const ast::Node& node) const;
  Value* TryValueOf(const ast::Node& node) const;
  const Type& TypeOf(const ast::Node& node) const;
  Value& ValueOf(const ast::Node& node) const;

  // Registration
  void RegisterType(const ast::Node& node, const Type& type);
  void RegisterValue(const ast::Node& node, Value* value);

 private:
  Zone& zone() const;

  const std::unique_ptr<Factory> factory_;
  const AnalyzerSettings& settings_;
  const std::unique_ptr<TypeFactory> type_factory_;
  std::unique_ptr<TypeMap> type_map_;
  std::unique_ptr<ValueMap> value_map_;

  DISALLOW_COPY_AND_ASSIGN(Context);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_CONTEXT_H_
