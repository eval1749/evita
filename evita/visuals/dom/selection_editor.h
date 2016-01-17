// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_SELECTION_EDITOR_H_
#define EVITA_VISUALS_DOM_SELECTION_EDITOR_H_

#include "base/macros.h"

namespace visuals {

class Node;
class Selection;

//////////////////////////////////////////////////////////////////////
//
// SelectionEditor
//
class SelectionEditor final {
 public:
  SelectionEditor();
  ~SelectionEditor();

  void Clear(Selection* selection);
  void Collapse(Selection* selection, Node* node, int offset);
  void ExtendTo(Selection* selection, Node* node, int offset);

 private:
  DISALLOW_COPY_AND_ASSIGN(SelectionEditor);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_SELECTION_EDITOR_H_
