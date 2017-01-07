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

class Environment;
enum class ErrorCode;
class Factory;
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
  Environment& global_environment() const;

  // Query
  Environment& EnvironmentOf(const ast::Node& node) const;
  Value* TryValueOf(const ast::Node& node) const;
  Value& ValueOf(const ast::Node& node) const;

  // Factory
  Environment& NewEnvironment(Environment* outer, const ast::Node& node);

  // Registration
  Value& RegisterValue(const ast::Node& node, Value* value);

 private:
  Zone& zone() const;

  void InstallPrimitiveTypes();
  static Environment& NewGlobalEnvironment(Zone* zone);

  std::unordered_map<const ast::Node*, Environment*> environment_map_;
  Environment& global_environment_;
  std::unique_ptr<Factory> factory_;
  const AnalyzerSettings& settings_;
  std::unique_ptr<ValueMap> value_map_;

  DISALLOW_COPY_AND_ASSIGN(Context);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_CONTEXT_H_
