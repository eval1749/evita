// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_SIMPLE_BOX_TREE_H_
#define EVITA_VISUALS_LAYOUT_SIMPLE_BOX_TREE_H_

#include <memory>
#include <stack>

#include "base/macros.h"
#include "base/logging.h"
#include "evita/gc/member.h"
#include "evita/visuals/view/public/view_lifecycle.h"

namespace visuals {

class Box;
class ContainerBox;
class Document;
class ViewLifecycle;
class RootBox;

namespace css {
class Style;
}

//////////////////////////////////////////////////////////////////////
//
// SimpleBoxTree
//
class SimpleBoxTree final {
 public:
  explicit SimpleBoxTree(const Document& document);
  SimpleBoxTree();
  ~SimpleBoxTree();

  RootBox* root_box() { return root_box_.get(); }

  template <typename T, typename... Args>
  SimpleBoxTree& Add(Args&&... args) {
    static_assert(std::is_base_of<Box, T>::value, "Box should be base of T");
    return AddInternal(
        std::move(std::unique_ptr<Box>(new T(root_box(), args...))));
  }

  template <typename T, typename... Args>
  SimpleBoxTree& Begin(Args&&... args) {
    static_assert(std::is_base_of<Box, T>::value, "Box should be base of T");
    return BeginInternal(
        std::move(std::unique_ptr<Box>(new T(root_box(), args...))));
  }

  template <typename T>
  SimpleBoxTree& End() {
    DCHECK(boxes_.top()->is<T>());
    return EndInternal();
  }

  void Finish();

  // Box
  SimpleBoxTree& SetStyle(const css::Style& style);

  // TextBox
  SimpleBoxTree& SetBaseline(float baseline);

 private:
  SimpleBoxTree& AddInternal(std::unique_ptr<Box> box);
  SimpleBoxTree& BeginInternal(std::unique_ptr<Box> box);
  SimpleBoxTree& EndInternal();
  void FinishInternal(Box* box);

  std::stack<Box*> boxes_;
  const Document& document_;
  std::unique_ptr<ViewLifecycle> lifecycle_;
  std::unique_ptr<ViewLifecycle::Scope> lifecycle_scope_;
  gc::Member<Document> own_document_;
  std::unique_ptr<RootBox> root_box_;

  DISALLOW_COPY_AND_ASSIGN(SimpleBoxTree);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_SIMPLE_BOX_TREE_H_
