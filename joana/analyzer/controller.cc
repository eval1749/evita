// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "joana/analyzer/controller.h"

#include "base/command_line.h"
#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/environment.h"
#include "joana/analyzer/environment_builder.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/print_as_tree.h"
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
  ostream << std::endl;
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
  const Value* value;
};

std::ostream& operator<<(std::ostream& ostream, const Dump& dump) {
  const auto& value = *dump.value;
  const auto depth = dump.depth;
  ostream << Indent{depth + 1} << value;
  if (auto* variable = value.TryAs<Variable>()) {
    std::cout << Indent{depth + 2} << "Assignments";
    for (const auto& assignment : variable->assignments())
      std::cout << Indent{depth + 3} << assignment;
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

const ast::Node& Controller::built_in_module() const {
  return BuiltInWorld::GetInstance()->global_module();
}

Factory& Controller::factory() const {
  return context_->factory();
}

void Controller::Analyze() {
  TypeChecker type_checker(context_.get());
  for (const auto& node : nodes_)
    type_checker.RunOn(*node);

  const auto* const command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch("dump_after_type"))
    DumpValues();
  if (command_line->HasSwitch("print_after_type"))
    PrintTree();
}

void Controller::DumpValues() {
  for (const auto& toplevel : nodes_) {
    for (const auto& node : ast::NodeTraversal::DescendantsOf(*toplevel)) {
      const auto* const value = context_->TryValueOf(node);
      if (!value)
        continue;
      std::cout << node << Dump{1, value} << std::endl;
    }
  }
}

void Controller::PrintTree() {
  for (const auto& toplevel : nodes_)
    std::cout << AsPrintableTree(*context_, *toplevel) << std::endl;
}

void Controller::Load(const ast::Node& node) {
  nodes_.push_back(&node);
  EnvironmentBuilder builder(context_.get());
  builder.RunOn(node);

  const auto* const command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch("dump_after_build"))
    DumpValues();
  if (command_line->HasSwitch("print_after_build"))
    PrintTree();
}

}  // namespace analyzer
}  // namespace joana
