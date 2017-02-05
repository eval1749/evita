// Copyright (c) 2017 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include "joana/analyzer/class_tree_builder.h"

#include "joana/analyzer/error_codes.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/value_editor.h"
#include "joana/analyzer/values.h"
#include "joana/base/graphs/graph_editor.h"
#include "joana/base/graphs/graph_sorter.h"

namespace joana {
namespace analyzer {

//
// ClassTreeBuilder::ClassGraph
//
ClassTreeBuilder::ClassGraph::ClassGraph() = default;
ClassTreeBuilder::ClassGraph::~ClassGraph() = default;

//
// ClassTreeBuilder::ClassNode
//
ClassTreeBuilder::ClassNode::ClassNode(Zone* zone, Class* clazz)
    : GraphNodeBase(zone), class_(*clazz) {}

ClassTreeBuilder::ClassNode::~ClassNode() = default;

//
// ClassTreeBuilder
//
ClassTreeBuilder::ClassTreeBuilder(Context* context)
    : ContextUser(context), zone_("ClassTreeBuilder") {}

ClassTreeBuilder::~ClassTreeBuilder() = default;

// public
void ClassTreeBuilder::Build() {
  for (const auto& node : graph_.nodes())
    TryFinalizeClass(&node->value());
  ValidateClassTree();
}

void ClassTreeBuilder::Process(Class* derived_class) {
  DCHECK(!derived_class->is_finalized()) << *derived_class;
  const auto& emplaced = processed_classes_.emplace(derived_class);
  DCHECK(emplaced.second) << derived_class;
  std::vector<Class*> pending_classes;
  for (auto& base_class : derived_class->base_classes()) {
    if (base_class.is_finalized())
      continue;
    pending_classes.push_back(&base_class);
  }
  if (pending_classes.empty()) {
    TryFinalizeClass(derived_class);
    DCHECK(derived_class->is_finalized()) << derived_class;
    return;
  }
  auto& derived_class_node = GetOrNewNode(derived_class);
  for (const auto& base_class : pending_classes) {
    auto& base_class_node = GetOrNewNode(base_class);
    ClassGraph::Editor().AddEdge(&graph_, &base_class_node,
                                 &derived_class_node);
  }
}

// private
ClassTreeBuilder::ClassNode& ClassTreeBuilder::GetOrNewNode(Class* clazz) {
  const auto& it = class_map_.find(clazz);
  if (it != class_map_.end())
    return *it->second;
  auto& new_node = *new (&zone_) ClassNode(&zone_, clazz);
  const auto& result = class_map_.emplace(clazz, &new_node);
  DCHECK(result.second);
  ClassGraph::Editor().AppendNode(&graph_, &new_node);
  return new_node;
}

bool ClassTreeBuilder::IsProcessed(const Class& clazz) const {
  if (processed_classes_.count(&clazz) == 1)
    return true;
  if (!clazz.Is<ConstructedClass>())
    return false;
  return IsProcessed(clazz.As<ConstructedClass>().generic_class());
}

void ClassTreeBuilder::TryFinalizeClass(Class* clazz) {
  if (clazz->is_finalized() || !IsProcessed(*clazz))
    return;
  for (auto& base_class : clazz->base_classes()) {
    if (!IsProcessed(base_class))
      return;
  }
  std::vector<Class*> class_list;
  std::unordered_set<const Class*> presents;
  // Make class list in Breadth-First-Search order.
  class_list.push_back(clazz);
  for (size_t index = 0; index < class_list.size(); ++index) {
    for (auto& base_class : class_list[index]->base_classes()) {
      DCHECK(IsProcessed(base_class)) << base_class;
      if (!base_class.is_finalized())
        return;
      if (presents.count(&base_class) > 0)
        continue;
      class_list.push_back(&base_class);
      presents.insert(&base_class);
    }
  }
  Value::Editor().SetClassList(clazz, class_list);
  auto& class_node = GetOrNewNode(clazz);
  ClassGraph::Editor().RemoveNode(&graph_, &class_node);
  std::vector<ClassNode*> successors(class_node.successors().begin(),
                                     class_node.successors().end());
  for (const auto& successor : successors) {
    ClassGraph::Editor().RemoveEdge(&graph_, &class_node, successor);
    if (successor->HasPredecessor())
      continue;
    TryFinalizeClass(&successor->value());
  }
}

void ClassTreeBuilder::ValidateClassTree() {
  std::set<std::pair<Class*, Class*>> cycles;
  for (const auto& node : graph_.nodes()) {
    auto& user_class = node->value();
    DCHECK(!user_class.is_finalized()) << user_class;
    if (!IsProcessed(user_class)) {
      AddError(user_class.node(), ErrorCode::CLASS_TREE_UNDEFINED_CLASS);
      continue;
    }
    for (const auto& successor : node->successors()) {
      auto& using_class = successor->value();
      DCHECK(IsProcessed(using_class)) << using_class << "<-" << user_class;
      DCHECK(!using_class.is_finalized()) << using_class;
      if (!successor->HasPredecessor())
        continue;
      auto key = user_class.id() < using_class.id()
                     ? std::make_pair(&user_class, &using_class)
                     : std::make_pair(&using_class, &user_class);
      const auto& result = cycles.insert(key);
      if (!result.second)
        continue;
      AddError(user_class.node(), ErrorCode::CLASS_TREE_CYCLE,
               using_class.node());
    }
  }
}

}  // namespace analyzer
}  // namespace joana
