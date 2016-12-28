// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_FACTORY_H_
#define JOANA_ANALYZER_FACTORY_H_

#include <memory>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "joana/base/memory/zone.h"

namespace joana {
namespace ast {
class Node;
}
namespace analyzer {

class Environment;
class Value;

//
// Factory
//
class Factory final {
 public:
  // |zone| A zone to store analyze results.
  explicit Factory(Zone* zone);
  ~Factory();

  Environment* global_environment() const { return global_environment_; }

  Environment* NewEnvironment(Environment* outer, const ast::Node& owner);
  Value* NewFunction(const ast::Node& node);
  Value* NewProperty(const ast::Node& node);
  Value* NewVariable(const ast::Node& node);

 private:
  static Environment* NewGlobalEnvironment(Zone* zone);

  Environment* const global_environment_;
  Zone& zone_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_FACTORY_H_
