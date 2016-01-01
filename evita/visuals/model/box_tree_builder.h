// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_BOX_TREE_BUILDER_H_
#define EVITA_VISUALS_MODEL_BOX_TREE_BUILDER_H_

#include <memory>
#include <stack>

#include "base/macros.h"

namespace visuals {

class Box;
class ContainerBox;
class RootBox;

namespace css {
class Style;
}

//////////////////////////////////////////////////////////////////////
//
// BoxTreeBuilder
//
class BoxTreeBuilder final {
 public:
  explicit BoxTreeBuilder(ContainerBox* container);
  BoxTreeBuilder();
  ~BoxTreeBuilder();

  std::unique_ptr<RootBox> Build();

  template <typename T, typename... Args>
  BoxTreeBuilder& Add(Args&&... args) {
    static_assert(std::is_base_of<Box, T>::value, "Box should be base of T");
    return AddInternal(
        std::move(std::unique_ptr<Box>(new T(root_box_, args...))));
  }

  template <typename T, typename... Args>
  BoxTreeBuilder& Begin(Args&&... args) {
    static_assert(std::is_base_of<Box, T>::value, "Box should be base of T");
    return BeginInternal(
        std::move(std::unique_ptr<Box>(new T(root_box_, args...))));
  }

  template <typename T>
  BoxTreeBuilder& End() {
    DCHECK(boxes_.top()->is<T>());
    return EndInternal();
  }

  template <typename T>
  void Finish(T* box) {
    static_assert(std::is_base_of<Box, T>::value, "Box should be base of T");
    FinishInternal(box);
  }

  // Box
  BoxTreeBuilder& SetStyle(const css::Style& style);

  // TextBox
  BoxTreeBuilder& SetBaseline(float baseline);

 private:
  BoxTreeBuilder& AddInternal(std::unique_ptr<Box> box);
  BoxTreeBuilder& BeginInternal(std::unique_ptr<Box> box);
  BoxTreeBuilder& EndInternal();
  void FinishInternal(Box* box);

  std::stack<Box*> boxes_;
  std::unique_ptr<RootBox> new_root_box_;
  RootBox* const root_box_;

  DISALLOW_COPY_AND_ASSIGN(BoxTreeBuilder);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_BOX_TREE_BUILDER_H_
