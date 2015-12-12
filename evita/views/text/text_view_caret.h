// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_TEXT_VIEW_CARET_H_
#define EVITA_VIEWS_TEXT_TEXT_VIEW_CARET_H_

#include "base/macros.h"
#include "evita/gfx/forward.h"
#include "evita/ui/caret.h"


namespace views {

//////////////////////////////////////////////////////////////////////
//
// TextViewCaret manages caret blinking.
//
class TextViewCaret : public ui::Caret {
 public:
  explicit TextViewCaret(ui::CaretOwner* owner);
  ~TextViewCaret() final;

 private:
  // ui::Caret members
  void Paint(gfx::Canvas* canvas, const gfx::RectF& bounds) final;

  DISALLOW_COPY_AND_ASSIGN(TextViewCaret);
};

}  // namespace views

#endif  // EVITA_VIEWS_TEXT_TEXT_VIEW_CARET_H_
