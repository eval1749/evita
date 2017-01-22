// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_ANNOTATION_H_
#define JOANA_ANALYZER_ANNOTATION_H_

#include <vector>

namespace joana {

namespace ast {
class Node;
}

namespace analyzer {

enum class ClassKind;
enum class Visibility;

//
// Annotation
//
class Annotation final {
 public:
  enum class Kind {
    Constructor,
    Define,
    Dict,
    Enum,
    Function,
    Interface,
    None,
    Type,
    TypeDef,
  };
  class Compiler;

  Annotation(Annotation&& other);
  Annotation();
  ~Annotation();

  const ast::Node* access_tag() const { return access_tag_; }
  const ast::Node* const_tag() const { return const_tag_; }
  const std::vector<const ast::Node*>& extends_tags() const {
    return extends_tags_;
  }
  const ast::Node* final_tag() const { return final_tag_; }
  const std::vector<const ast::Node*>& implements_tags() const {
    return implements_tags_;
  }
  const ast::Node* kind_tag() const { return kind_tag_; }
  const ast::Node* override_tag() const { return override_tag_; }
  const std::vector<const ast::Node*>& parameter_tags() const {
    return parameter_tags_;
  }
  const ast::Node* return_tag() const { return return_tag_; }
  const ast::Node* this_tag() const { return this_tag_; }
  const ast::Node* type_node() const { return type_node_; }
  const std::vector<const ast::Node*>& type_parameter_names() const {
    return type_parameter_names_;
  }

  ClassKind class_kind() const;
  const ast::Node& document() const;
  bool has_document() const { return document_ != nullptr; }
  bool is_constructor() const { return kind_ == Kind::Constructor; }
  bool is_enum() const { return kind_ == Kind::Enum; }
  bool is_function() const { return kind_ == Kind::Function; }
  bool is_interface() const { return kind_ == Kind::Interface; }
  bool is_none() const { return kind_ == Kind::None; }
  bool is_type() const { return kind_ == Kind::Type; }
  Kind kind() const { return kind_; }
  Visibility visibility() const;

 private:
  const ast::Node* document_ = nullptr;
  Kind kind_ = Kind::None;

  // Tags
  const ast::Node* access_tag_ = nullptr;
  const ast::Node* const_tag_ = nullptr;

  // Class can have at most one @extends tag and @interface can have multiple
  // @extends tags.
  std::vector<const ast::Node*> extends_tags_;

  const ast::Node* final_tag_ = nullptr;
  std::vector<const ast::Node*> implements_tags_;
  const ast::Node* kind_tag_ = nullptr;
  const ast::Node* override_tag_ = nullptr;
  std::vector<const ast::Node*> parameter_tags_;
  const ast::Node* return_tag_ = nullptr;
  const ast::Node* this_tag_ = nullptr;
  const ast::Node* type_node_ = nullptr;

  // List of |TypeName| extracted from @template tags.
  std::vector<const ast::Node*> type_parameter_names_;
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_ANNOTATION_H_
