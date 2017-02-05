// Copyright (c) 2017 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include "joana/analyzer/class_tree_builder.h"

#include "joana/analyzer/error_codes.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/value_editor.h"
#include "joana/analyzer/values.h"
#include "joana/base/graphs/graph_editor.h"
#include "joana/base/graphs/graph_sorter.h"

namespace joana {
namespace analyzer {

namespace {

std::unordered_map<const TypeParameter*, const Type*> ComputeArgumentMap(
    const Class& clazz) {
  std::unordered_map<const TypeParameter*, const Type*> argument_map;
  if (!clazz.Is<ConstructedClass>())
    return argument_map;
  const auto& constructed_class = clazz.As<ConstructedClass>();
  auto it = constructed_class.arguments().begin();
  for (const auto& type_parameter :
       constructed_class.generic_class().parameters()) {
    const auto& emplaced = argument_map.emplace(&type_parameter, &*it);
    DCHECK(emplaced.second);
    ++it;
  }
  return argument_map;
}

bool IsFinalized(const Class& clazz) {
  if (clazz.Is<ConstructedClass>())
    return clazz.As<ConstructedClass>().generic_class().is_finalized();
  return clazz.is_finalized();
}

}  // namespace

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
  for (const auto& node : graph_.nodes()) {
    auto& clazz = node->value();
    if (!CanFinalize(clazz))
      continue;
    DCHECK(node->HasPredecessor()) << clazz;
    FinalizeClass(&clazz);
  }
  for (const auto& constructed_class : pending_constructed_classes_) {
    if (constructed_class->is_finalized())
      continue;
    FinalizeConstructedClass(constructed_class);
    DCHECK(constructed_class->is_finalized()) << constructed_class;
  }
  ValidateClassTree();
}

void ClassTreeBuilder::ProcessClassDefinition(Class* derived_class) {
  DCHECK(!derived_class->Is<ConstructedClass>()) << derived_class;
  DCHECK(!IsFinalized(*derived_class)) << *derived_class;
  const auto& emplaced = processed_classes_.emplace(derived_class);
  DCHECK(emplaced.second) << derived_class;
  std::unordered_set<Class*> pending_classes;
  for (auto& original_base_class : derived_class->base_classes()) {
    auto& base_class =
        original_base_class.Is<ConstructedClass>()
            ? original_base_class.As<ConstructedClass>().generic_class()
            : original_base_class;
    if (ProcessClassReference(&base_class))
      continue;
    pending_classes.insert(&base_class);
  }
  if (pending_classes.empty()) {
    FinalizeClass(derived_class);
    DCHECK(IsFinalized(*derived_class)) << *derived_class;
    return;
  }
  auto& derived_class_node = GetOrNewNode(derived_class);
  for (const auto& base_class : pending_classes) {
    auto& base_class_node = GetOrNewNode(base_class);
    ClassGraph::Editor().AddEdge(&graph_, &base_class_node,
                                 &derived_class_node);
  }
}

bool ClassTreeBuilder::ProcessClassReference(Class* clazz) {
  if (!clazz->Is<ConstructedClass>())
    return clazz->is_finalized();
  auto& constructed_class = clazz->As<ConstructedClass>();
  if (constructed_class.generic_class().is_finalized()) {
    FinalizeConstructedClass(&constructed_class);
    return true;
  }
  pending_constructed_classes_.insert(&constructed_class);
  return false;
}

// private
bool ClassTreeBuilder::CanFinalize(const Class& clazz) const {
  if (clazz.Is<ConstructedClass>())
    return CanFinalize(clazz.As<ConstructedClass>().generic_class());
  if (IsFinalized(clazz) || !IsProcessed(clazz))
    return false;
  for (const auto& original_base_class : clazz.base_classes()) {
    const auto& base_class =
        original_base_class.Is<ConstructedClass>()
            ? original_base_class.As<ConstructedClass>().generic_class()
            : original_base_class;
    if (!base_class.is_finalized() || !IsProcessed(base_class))
      return false;
  }
  return true;
}

Class& ClassTreeBuilder::ConstructClass(Class* clazz,
                                        const ArgumentMap& argument_map) {
  DCHECK(!clazz->Is<GenericClass>()) << clazz;
  if (!clazz->Is<ConstructedClass>()) {
    DCHECK(clazz->is_finalized()) << clazz;
    return *clazz;
  }
  DCHECK(clazz->As<ConstructedClass>().generic_class().is_finalized()) << clazz;
  std::vector<const Type*> arguments;
  auto has_parameters = false;
  for (const auto& argument : clazz->As<ConstructedClass>().arguments()) {
    if (!argument.Is<TypeParameter>()) {
      arguments.push_back(&argument);
      continue;
    }
    const auto& it = argument_map.find(&argument.As<TypeParameter>());
    if (it == argument_map.end()) {
      arguments.push_back(&argument);
      continue;
    }
    arguments.push_back(it->second);
    has_parameters = true;
  }
  if (!has_parameters) {
    if (clazz->is_finalized())
      return *clazz;
    FinalizeConstructedClass(&clazz->As<ConstructedClass>());
    return *clazz;
  }
  auto& constructed_class = factory().NewConstructedClass(
      &clazz->As<ConstructedClass>().generic_class(), arguments);
  if (constructed_class.is_finalized())
    return constructed_class;
  FinalizeConstructedClass(&constructed_class.As<ConstructedClass>());
  return constructed_class;
}

void ClassTreeBuilder::FinalizeClass(Class* clazz) {
  DCHECK(!clazz->Is<ConstructedClass>()) << clazz;
  DCHECK(CanFinalize(*clazz)) << clazz;
  // Make class list in Breadth-First-Search order.
  std::vector<Class*> class_list;
  std::unordered_set<const Class*> presents;
  class_list.push_back(clazz);
  for (size_t index = 0; index < class_list.size(); ++index) {
    for (auto& base_class : class_list[index]->base_classes()) {
      if (base_class.Is<ConstructedClass>() && !base_class.is_finalized())
        FinalizeConstructedClass(&base_class.As<ConstructedClass>());
      DCHECK(base_class.is_finalized()) << base_class << " in " << clazz;
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
    auto& derived_class = successor->value();
    if (derived_class.Is<ConstructedClass>()) {
      FinalizeConstructedClass(&derived_class.As<ConstructedClass>());
      continue;
    }
    FinalizeClass(&derived_class);
  }
}

void ClassTreeBuilder::FinalizeConstructedClass(
    ConstructedClass* constructed_class) {
  auto& generic_class = constructed_class->generic_class();
  DCHECK(generic_class.is_finalized()) << constructed_class;
  const auto& argument_map = ComputeArgumentMap(*constructed_class);
  std::vector<Class*> class_list;
  class_list.push_back(constructed_class);
  for (auto& base_class : generic_class.class_list()) {
    if (base_class == generic_class)
      continue;
    class_list.push_back(&ConstructClass(&base_class, argument_map));
  }
  Value::Editor().SetClassList(constructed_class, class_list);
}

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

void ClassTreeBuilder::ValidateClassTree() {
  std::set<std::pair<Class*, Class*>> cycles;
  for (const auto& node : graph_.nodes()) {
    auto& user_class = node->value();
    if (user_class.Is<ConstructedClass>())
      continue;
    DCHECK(!IsFinalized(user_class)) << user_class;
    if (!IsProcessed(user_class)) {
      AddError(user_class.node(), ErrorCode::CLASS_TREE_UNDEFINED_CLASS);
      continue;
    }
    for (const auto& successor : node->successors()) {
      auto& using_class = successor->value();
      DCHECK(IsProcessed(using_class)) << using_class << "<-" << user_class;
      DCHECK(!IsFinalized(using_class)) << using_class;
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
