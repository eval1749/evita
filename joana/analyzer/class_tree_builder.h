// Copyright (c) 2017 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_CLASS_TREE_BUILDER_H_
#define JOANA_ANALYZER_CLASS_TREE_BUILDER_H_

#include <unordered_map>
#include <unordered_set>

#include "base/macros.h"
#include "joana/analyzer/context_user.h"
#include "joana/base/graphs/graph.h"
#include "joana/base/memory/zone.h"
#include "joana/base/memory/zone_allocated.h"

namespace joana {
namespace analyzer {

class Class;
class ConstructedClass;
class Type;
class TypeParameter;

//
// ClassTreeBuilder
//
class ClassTreeBuilder final : public ContextUser {
 public:
  explicit ClassTreeBuilder(Context* context);
  ~ClassTreeBuilder();

  void Build();
  void ProcessClassDefinition(Class* clazz);

  // Returns true if |clazz| is finalized.
  bool ProcessClassReference(Class* clazz);

 private:
  class ClassNode;

  class ClassGraph final : public Graph<ClassTreeBuilder, ClassNode> {
   public:
    ClassGraph();
    ~ClassGraph();

   private:
    DISALLOW_COPY_AND_ASSIGN(ClassGraph);
  };

  class ClassNode : public ClassGraph::GraphNodeBase, public ZoneAllocated {
   public:
    ClassNode(Zone* zone, Class* clazz);
    ~ClassNode();

    Class& value() const { return class_; }

   private:
    Class& class_;

    DISALLOW_COPY_AND_ASSIGN(ClassNode);
  };

  using ArgumentMap = std::unordered_map<const TypeParameter*, const Type*>;

  bool CanFinalize(const Class& clazz) const;
  Class& ConstructClass(Class* base_class, const ArgumentMap& argument_map);
  void FinalizeClass(Class* clazz);
  void FinalizeConstructedClass(ConstructedClass* clazz);
  ClassNode& GetOrNewNode(Class* clazz);
  bool IsProcessed(const Class& clazz) const;
  void ValidateClassTree();

  std::unordered_set<ConstructedClass*> pending_constructed_classes_;

  // A set of classes passed in |Process()|.
  std::unordered_set<const Class*> processed_classes_;

  // Mapping from |Class| to |ClassNode|.
  std::unordered_map<Class*, ClassNode*> class_map_;

  // Holds non-finalized classes in directional graph, each vertex is a
  // |ClassNode| and each edge is user class to using class.
  ClassGraph graph_;

  // A |Zone| for holding |ClassGraph|.
  Zone zone_;

  DISALLOW_COPY_AND_ASSIGN(ClassTreeBuilder);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_CLASS_TREE_BUILDER_H_
