// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_SIMPLE_BOX_TREE_BUILDER_H_
#define EVITA_VISUALS_MODEL_SIMPLE_BOX_TREE_BUILDER_H_

#include <memory>
#include <stack>

#include "base/macros.h"
#include "base/logging.h"

namespace visuals {

class Box;
class ContainerBox;
class RootBox;

namespace css {
class Style;
}

//////////////////////////////////////////////////////////////////////
//
// SimpleBoxTreeBuilder
//
class SimpleBoxTreeBuilder final {
 public:
  explicit SimpleBoxTreeBuilder(ContainerBox* container);
  SimpleBoxTreeBuilder();
  ~SimpleBoxTreeBuilder();

  std::unique_ptr<RootBox> Build();

  template <typename T, typename... Args>
  SimpleBoxTreeBuilder& Add(Args&&... args) {
    static_assert(std::is_base_of<Box, T>::value, "Box should be base of T");
    return AddInternal(
        std::move(std::unique_ptr<Box>(new T(root_box_, args...))));
  }

  template <typename T, typename... Args>
  SimpleBoxTreeBuilder& Begin(Args&&... args) {
    static_assert(std::is_base_of<Box, T>::value, "Box should be base of T");
    return BeginInternal(
        std::move(std::unique_ptr<Box>(new T(root_box_, args...))));
  }

  template <typename T>
  SimpleBoxTreeBuilder& End() {
    DCHECK(boxes_.top()->is<T>());
    return EndInternal();
  }

  template <typename T>
  void Finish(T* box) {
    static_assert(std::is_base_of<Box, T>::value, "Box should be base of T");
    FinishInternal(box);
  }

  // Box
  SimpleBoxTreeBuilder& SetStyle(const css::Style& style);

  // TextBox
  SimpleBoxTreeBuilder& SetBaseline(float baseline);

 private:
  SimpleBoxTreeBuilder& AddInternal(std::unique_ptr<Box> box);
  SimpleBoxTreeBuilder& BeginInternal(std::unique_ptr<Box> box);
  SimpleBoxTreeBuilder& EndInternal();
  void FinishInternal(Box* box);

  std::stack<Box*> boxes_;
  std::unique_ptr<RootBox> new_root_box_;
  RootBox* const root_box_;

  DISALLOW_COPY_AND_ASSIGN(SimpleBoxTreeBuilder);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_SIMPLE_BOX_TREE_BUILDER_H_
