// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_CONTROLLER_H_
#define JOANA_ANALYZER_CONTROLLER_H_

#include <memory>

#include "base/macros.h"

namespace joana {

namespace ast {
class Node;
}

class AnalyzerSettings;

namespace analyzer {

class Context;

//
// Controller
//
class Controller final {
 public:
  explicit Controller(const AnalyzerSettings& settings);
  ~Controller();

  void Analyze();
  void Load(const ast::Node& node);

 private:
  const std::unique_ptr<Context> context_;

  DISALLOW_COPY_AND_ASSIGN(Controller);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_CONTROLLER_H_
