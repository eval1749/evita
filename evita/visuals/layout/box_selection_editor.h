// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_SELECTION_EDITOR_H_
#define EVITA_VISUALS_LAYOUT_BOX_SELECTION_EDITOR_H_

#include "base/macros.h"
#include "evita/visuals/css/values.h"

namespace gfx {
class FloatColor;
}

namespace visuals {

class Box;
class BoxSelection;

//////////////////////////////////////////////////////////////////////
//
// BoxSelectionEditor
//
class BoxSelectionEditor final {
 public:
  BoxSelectionEditor();
  ~BoxSelectionEditor();

  void Clear(BoxSelection* selection);
  void Collapse(BoxSelection* selection, Box* box, int offset);
  void ExtendTo(BoxSelection* selection, Box* box, int offset);
  void SetCaretColor(BoxSelection* selection, const gfx::FloatColor& color);
  void SetCaretShape(BoxSelection* selection, const css::CaretShape& shape);
  void SetSelectionColor(BoxSelection* selection, const gfx::FloatColor& color);

 private:
  DISALLOW_COPY_AND_ASSIGN(BoxSelectionEditor);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_SELECTION_EDITOR_H_
