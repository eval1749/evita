// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CARET_OWNER_H_
#define EVITA_UI_CARET_OWNER_H_

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
 public:
  virtual void DidFireCaretTimer() = 0;

 protected:
  CaretOwner();
  virtual ~CaretOwner();

 private:
  DISALLOW_COPY_AND_ASSIGN(CaretOwner);
};

}  // namespace ui

#endif  // EVITA_UI_CARET_OWNER_H_
