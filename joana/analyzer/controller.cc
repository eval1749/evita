// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "joana/analyzer/controller.h"

#include "joana/analyzer/context.h"
#include "joana/analyzer/environment.h"
#include "joana/analyzer/environment_builder.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/type_checker.h"
#include "joana/analyzer/values.h"
#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/syntax.h"

namespace joana {
namespace analyzer {

namespace {

//
// Indent
//
struct Indent {
  int depth;
};

std::ostream& operator<<(std::ostream& ostream, const Indent& indent) {
  if (indent.depth == 0)
    return ostream;
  for (auto counter = 0; counter < indent.depth - 1; ++counter)
    ostream << "|  ";
  return ostream << "+--";
}

//
// Dump
//
struct Dump {
  int depth;
  const Environment* environment;
};

std::ostream& operator<<(std::ostream& ostream, const Dump& dump) {
  const auto& environment = *dump.environment;
  const auto depth = dump.depth;
  ostream << Indent{depth} << "Environment " << environment.owner()
          << std::endl;
  for (const auto& name : environment.names_for_testing()) {
    const auto& value = *environment.TryValueOf(*name);
    ostream << Indent{depth + 1} << value << std::endl;
    if (auto* variable = value.TryAs<Variable>()) {
      for (const auto& assignment : variable->assignments())
        std::cout << Indent{depth + 2} << assignment << std::endl;
    }
  }
  return ostream;
}

}  // namespace

//
// Controller
//
Controller::Controller(const AnalyzerSettings& settings)
    : context_(new Context(settings)) {}

Controller::~Controller() = default;

Factory& Controller::factory() const {
  return context_->factory();
}

void Controller::Analyze() {
  for (const auto& toplevel : nodes_) {
    for (const auto& node : ast::NodeTraversal::DescendantsOf(*toplevel)) {
      const auto* const value = context_->TryValueOf(node);
      if (!value)
        continue;
      std::cout << node << " = " << value << std::endl;
      if (auto* variable = value->TryAs<Variable>()) {
        for (const auto& assignment : variable->assignments())
          std::cout << "+--" << assignment << std::endl;
      }
    }
  }

  TypeChecker type_checker(context_.get());
  for (const auto& node : nodes_)
    type_checker.RunOn(*node);

  std::cout << std::endl << "Dump global environments" << std::endl;
  std::cout << Dump{0, &context_->global_environment()};

  auto counter = 0;
  for (const auto& node : nodes_) {
    const auto& environment = context_->EnvironmentOf(*node);
    if (!environment.outer())
      continue;
    if (++counter == 1)
      std::cout << std::endl << "Dump module environments" << std::endl;
    std::cout << Dump{0, &environment};
  }
}

void Controller::Load(const ast::Node& node) {
  nodes_.push_back(&node);
  EnvironmentBuilder builder(context_.get());
  builder.RunOn(node);
}

}  // namespace analyzer
}  // namespace joana
