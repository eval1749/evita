// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_EDITOR_H_
#define EVITA_VISUALS_LAYOUT_BOX_EDITOR_H_

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "evita/visuals/layout/box_forward.h"

namespace visuals {

class BoxSelection;
class FloatColor;
class FloatRect;
class FloatSize;
class FontDescription;
class ShapeData;
class TextFormat;

namespace css {
class Display;
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
  void AppendChild(ContainerBox* container, Box* new_child);
  void MarkDirty(Box* box);
  void RemoveAllChildren(ContainerBox* container_box);
  void RemoveChild(ContainerBox* container, Box* old_child);
  void RemoveDescendants(ContainerBox* container_box);
  void SetStyle(Box* box, const css::Style& style);

  void SetContentChanged(ContentBox* box);
  void SetDisplay(Box* box, const css::Display& display);
  void SetShouldPaint(Box* box);
  void WillDestroy(Box* box);

  // ShapeBox
  void SetShapeData(ShapeBox* box, const ShapeData& data);

  // TextBox
  void AllocateTextLayout(TextBox* box);
  const TextFormat& EnsureTextFormat(TextBox* box);
  void SetBaseline(TextBox* box, float new_baseline);
  void SetTextData(TextBox* box, const base::StringPiece16& data);

  // RootBox
  void ScheduleForcePaint(RootBox* root_box);
  void SetSelection(RootBox* root_box, const BoxSelection& selection);
  void SetViewportSize(RootBox* root_box, const FloatSize& size);

 private:
  const FontDescription& ComputeFontDescription(const TextBox& box);
  void SetShapeStyle(ShapeBox* box, const css::Style& style);
  void SetTextColor(TextBox* box, const FloatColor& color);
  void SetTextStyle(TextBox* box, const css::Style& style);

  DISALLOW_COPY_AND_ASSIGN(BoxEditor);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_EDITOR_H_
