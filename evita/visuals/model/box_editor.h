// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_BOX_EDITOR_H_
#define EVITA_VISUALS_MODEL_BOX_EDITOR_H_

#include <memory>

#include "base/macros.h"

namespace visuals {

class Box;
class ContainerBox;
class FloatColor;
class FloatRect;
class FloatSize;
class InlineBox;
class RootBox;
class TextBox;

namespace css {
class Style;
}

//////////////////////////////////////////////////////////////////////
//
// BoxEditor
//
class BoxEditor final {
 public:
  BoxEditor();
  ~BoxEditor();

  // Box
  void DidLayout(Box* box);
  void DidMove(Box* box);
  void DidPaint(Box* box);
  void SetBounds(Box* box, const FloatRect& new_bounds);
  void SetLayoutClean(Box* box);

  // ContainerBox
  template <typename T>
  T* AppendChild(ContainerBox* container, std::unique_ptr<T>&& child) {
    static_assert(std::is_base_of<Box, T>::value, "Box should be base of T");
    const auto& result = child.get();
    AppendChild(container, std::unique_ptr<Box>(child.release()));
    return result;
  }

  Box* AppendChild(ContainerBox* container, std::unique_ptr<Box> child);
  std::unique_ptr<Box> RemoveChild(ContainerBox* container, Box* child);
  void SetStyle(Box* box, const css::Style& style);

  void SetContentChanged(InlineBox* box);
  void SetShouldPaint(Box* box);
  void WillDestroy(Box* box);

  // TextBox
  void SetBaseline(TextBox* box, float new_baseline);
  void SetTextColor(TextBox* box, const FloatColor& color);

  // RootBox
  void SetViewportSize(RootBox* root_box, const FloatSize& size);

 private:
  void DidChangeChild(ContainerBox* container);
  void ScheduleVisualUpdateIfNeeded(Box* box);

  DISALLOW_COPY_AND_ASSIGN(BoxEditor);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_BOX_EDITOR_H_
