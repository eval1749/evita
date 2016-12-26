// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_TYPES_H_
#define JOANA_AST_TYPES_H_

#include <utility>
#include <vector>

#include "joana/ast/node.h"
#include "joana/ast/node_forward.h"
#include "joana/base/iterator_utils.h"

namespace joana {
namespace ast {

enum class FunctionTypeKind {
  Normal,
  This,
  New,
};

//
// Type
//
class JOANA_AST_EXPORT Type : public Node {
  DECLARE_ABSTRACT_AST_NODE(Type, Node);

 public:
  ~Type() override;

 protected:
  explicit Type(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(Type);
};

//
// TypeList
//
class JOANA_AST_EXPORT TypeList final : public ZoneAllocated {
 public:
  ~TypeList();

  auto begin() const { return ReferenceRangeOf(types_).begin(); }
  bool empty() const { return types_.empty(); }
  auto end() const { return ReferenceRangeOf(types_).end(); }
  size_t size() const { return types_.size(); }

 private:
  friend class NodeFactory;

  TypeList(Zone* zone, const std::vector<const Type*>& types);

  const ZoneVector<const Type*> types_;

  DISALLOW_COPY_AND_ASSIGN(TypeList);
};

//
// AnyType
//
class JOANA_AST_EXPORT AnyType final : public NodeTemplate<Type> {
  DECLARE_CONCRETE_AST_NODE(AnyType, Type);

 public:
  ~AnyType() final;

 private:
  explicit AnyType(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(AnyType);
};

//
// FunctionType
//
class JOANA_AST_EXPORT FunctionType final
    : public NodeTemplate<Type,
                          FunctionTypeKind,
                          const TypeList*,
                          const Type*> {
  DECLARE_CONCRETE_AST_NODE(FunctionType, Type);

 public:
  ~FunctionType() final;

  FunctionTypeKind kind() const { return member_at<0>(); }
  const TypeList& parameter_types() const { return *member_at<1>(); }
  const Type& return_type() const { return *member_at<2>(); }

 private:
  FunctionType(const SourceCodeRange& range,
               FunctionTypeKind kind,
               const TypeList& parameter_types,
               const Type& return_type);

  DISALLOW_COPY_AND_ASSIGN(FunctionType);
};

//
// InvalidType
//
class JOANA_AST_EXPORT InvalidType final : public NodeTemplate<Type> {
  DECLARE_CONCRETE_AST_NODE(InvalidType, Type);

 public:
  ~InvalidType() final;

 private:
  explicit InvalidType(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(InvalidType);
};

//
// NullableType
//
class JOANA_AST_EXPORT NullableType final
    : public NodeTemplate<Type, const Type*> {
  DECLARE_CONCRETE_AST_NODE(NullableType, Type);

 public:
  ~NullableType() final;

  const Type& type() const { return *member_at<0>(); }

 private:
  NullableType(const SourceCodeRange& range, const Type& type);

  DISALLOW_COPY_AND_ASSIGN(NullableType);
};

//
// NonNullableType
//
class JOANA_AST_EXPORT NonNullableType final
    : public NodeTemplate<Type, const Type*> {
  DECLARE_CONCRETE_AST_NODE(NonNullableType, Type);

 public:
  ~NonNullableType() final;

  const Type& type() const { return *member_at<0>(); }

 private:
  NonNullableType(const SourceCodeRange& range, const Type& type);

  DISALLOW_COPY_AND_ASSIGN(NonNullableType);
};

//
// OptionalType
//
class JOANA_AST_EXPORT OptionalType final
    : public NodeTemplate<Type, const Type*> {
  DECLARE_CONCRETE_AST_NODE(OptionalType, Type);

 public:
  ~OptionalType() final;

  const Type& type() const { return *member_at<0>(); }

 private:
  OptionalType(const SourceCodeRange& range, const Type& type);

  DISALLOW_COPY_AND_ASSIGN(OptionalType);
};

//
// RecordTypeMembers
//
class JOANA_AST_EXPORT RecordTypeMembers final : public ZoneAllocated {
  using Member = std::pair<const Name*, const Type*>;

 public:
  ~RecordTypeMembers();

  auto begin() const { return members_.begin(); }
  bool empty() const { return members_.empty(); }
  auto end() const { return members_.end(); }
  size_t size() const { return members_.size(); }

 private:
  friend class NodeFactory;

  RecordTypeMembers(Zone* zone, const std::vector<Member>& members);

  const ZoneVector<Member> members_;

  DISALLOW_COPY_AND_ASSIGN(RecordTypeMembers);
};

//
// RecordType
//
class JOANA_AST_EXPORT RecordType final
    : public NodeTemplate<Type, const RecordTypeMembers*> {
  DECLARE_CONCRETE_AST_NODE(RecordType, Type);

 public:
  ~RecordType() final;

  const RecordTypeMembers& members() const { return *member_at<0>(); }

 private:
  RecordType(const SourceCodeRange& range, const RecordTypeMembers& members);

  DISALLOW_COPY_AND_ASSIGN(RecordType);
};

//
// RestType
//
class JOANA_AST_EXPORT RestType final : public NodeTemplate<Type, const Type*> {
  DECLARE_CONCRETE_AST_NODE(RestType, Type);

 public:
  ~RestType() final;

  const Type& type() const { return *member_at<0>(); }

 private:
  RestType(const SourceCodeRange& range, const Type& type);

  DISALLOW_COPY_AND_ASSIGN(RestType);
};

//
// TupleType
//
class JOANA_AST_EXPORT TupleType final
    : public NodeTemplate<Type, const TypeList*> {
  DECLARE_CONCRETE_AST_NODE(TupleType, Type);

 public:
  ~TupleType() final;

  const TypeList& members() const { return *member_at<0>(); }

 private:
  TupleType(const SourceCodeRange& range, const TypeList& members);

  DISALLOW_COPY_AND_ASSIGN(TupleType);
};

//
// TypeApplication
//
class JOANA_AST_EXPORT TypeApplication final
    : public NodeTemplate<Type, const Name*, const TypeList*> {
  DECLARE_CONCRETE_AST_NODE(TypeApplication, Type);

 public:
  ~TypeApplication() final;

  const Name& name() const { return *member_at<0>(); }
  const TypeList& parameters() const { return *member_at<1>(); }

 private:
  TypeApplication(const SourceCodeRange& range,
                  const Name& name,
                  const TypeList& members);

  DISALLOW_COPY_AND_ASSIGN(TypeApplication);
};

//
// TypeGroup
//
class JOANA_AST_EXPORT TypeGroup final
    : public NodeTemplate<Type, const Type*> {
  DECLARE_CONCRETE_AST_NODE(TypeGroup, Type);

 public:
  ~TypeGroup() final;

  const Type& type() const { return *member_at<0>(); }

 private:
  TypeGroup(const SourceCodeRange& range, const Type& type);

  DISALLOW_COPY_AND_ASSIGN(TypeGroup);
};

//
// TypeName
//
class JOANA_AST_EXPORT TypeName final : public NodeTemplate<Type, const Name*> {
  DECLARE_CONCRETE_AST_NODE(TypeName, Type);

 public:
  ~TypeName() final;

  const Name& name() const { return *member_at<0>(); }

 private:
  TypeName(const SourceCodeRange& range, const Name& name);

  DISALLOW_COPY_AND_ASSIGN(TypeName);
};

//
// UnionType
//
class JOANA_AST_EXPORT UnionType final
    : public NodeTemplate<Type, const TypeList*> {
  DECLARE_CONCRETE_AST_NODE(UnionType, Type);

 public:
  ~UnionType() final;

  const TypeList& members() const { return *member_at<0>(); }

 private:
  UnionType(const SourceCodeRange& range, const TypeList& members);

  DISALLOW_COPY_AND_ASSIGN(UnionType);
};

//
// UnknownType
//
class JOANA_AST_EXPORT UnknownType final : public NodeTemplate<Type> {
  DECLARE_CONCRETE_AST_NODE(UnknownType, Type);

 public:
  ~UnknownType() final;

 private:
  explicit UnknownType(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(UnknownType);
};

//
// VoidType
//
class JOANA_AST_EXPORT VoidType final : public NodeTemplate<Type> {
  DECLARE_CONCRETE_AST_NODE(VoidType, Type);

 public:
  ~VoidType() final;

 private:
  explicit VoidType(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(VoidType);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_TYPES_H_
