// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <array>
#include <iostream>
#include <utility>

#include "joana/analyzer/controller.h"

#include "base/command_line.h"
#include "base/strings/string_split.h"
#include "joana/analyzer/built_in_world.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/name_resolver.h"
#include "joana/analyzer/print_as_tree.h"
#include "joana/analyzer/type_checker.h"
#include "joana/analyzer/type_resolver.h"
#include "joana/analyzer/values.h"
#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/syntax.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_range.h"

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

typedef std::unique_ptr<Pass> PassConstructor(Context* context);

struct PassEntry {
  PassConstructor* constructor;
  const char* key;
};

template <typename PassName>
std::unique_ptr<Pass> NewPass(Context* context) {
  return std::move(std::make_unique<PassName>(context));
}

const std::array<const PassEntry, 3> kPassList = {
    PassEntry{&NewPass<NameResolver>, "name"},
    PassEntry{&NewPass<TypeResolver>, "type"},
    PassEntry{&NewPass<TypeChecker>, "check"},
};

bool ShouldSkip(const ast::Node& toplevel) {
  const auto& source_code = toplevel.range().source_code();
  return base::StringPiece16(source_code.file_path().value())
      .starts_with(L"//externs/");
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
  const auto* const command_line = base::CommandLine::ForCurrentProcess();
  const auto& dump_list =
      base::SplitString(command_line->GetSwitchValueASCII("dump"), ",",
                        base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  const auto& print_list =
      base::SplitString(command_line->GetSwitchValueASCII("print"), ",",
                        base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  for (const auto& entry : kPassList) {
    const auto& pass = entry.constructor(context_.get());
    for (const auto& node : nodes_)
      pass->RunOn(*node);
    pass->RunOnAll();
    if (std::count(dump_list.begin(), dump_list.end(), entry.key) > 0)
      DumpValues();
    if (std::count(print_list.begin(), print_list.end(), entry.key) > 0)
      PrintTree();
  }
}

void Controller::DumpValues() {
  for (const auto& toplevel : nodes_) {
    if (ShouldSkip(*toplevel))
      continue;
    for (const auto& node : ast::NodeTraversal::DescendantsOf(*toplevel)) {
      const auto* const value = context_->TryValueOf(node);
      if (!value)
        continue;
      std::cout << node << Dump{1, value} << std::endl;
    }
  }
}

void Controller::PrintTree() {
  for (const auto& toplevel : nodes_) {
    if (ShouldSkip(*toplevel))
      continue;
    std::cout << AsPrintableTree(*context_, *toplevel) << std::endl;
  }
}

void Controller::Load(const ast::Node& node) {
  DCHECK(std::find(nodes_.begin(), nodes_.end(), &node) == nodes_.end())
      << "we should call Load() once for each node: " << node;
  nodes_.push_back(&node);
}

}  // namespace analyzer
}  // namespace joana
