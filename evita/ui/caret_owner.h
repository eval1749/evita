// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_caret_owner_h)
#define INCLUDE_evita_ui_caret_owner_h

#include "base/basictypes.h"
#include "evita/gfx/rect_f.h"

namespace gfx {
class Canvas;
}

namespace ui {

class Caret;

//////////////////////////////////////////////////////////////////////
//
// CaretOwner
//
class CaretOwner {
  protected: CaretOwner();
  protected: virtual ~CaretOwner();

  public: virtual void DidFireCaretTimer() = 0;

  DISALLOW_COPY_AND_ASSIGN(CaretOwner);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_caret_owner_h)
