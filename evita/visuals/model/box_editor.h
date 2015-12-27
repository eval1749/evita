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
class TextBox;

//////////////////////////////////////////////////////////////////////
//
// BoxEditor
//
class BoxEditor final {
 public:
  BoxEditor();
  ~BoxEditor();

  // Box
  void SetBounds(Box* box, const FloatRect& new_bounds);
  void SetLayoutClean(Box* box);
  void SetParent(Box* box, ContainerBox* parent);

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

  // TextBox
  void SetBaseline(TextBox* box, float new_baseline);
  void SetColor(TextBox* box, const FloatColor& new_color);

 private:
  void DidChangeContent(Box* box);
  void DidChangeLayout(Box* box);

  DISALLOW_COPY_AND_ASSIGN(BoxEditor);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_BOX_EDITOR_H_
