// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_FORMS_FORM_PAINT_INFO_H_
#define EVITA_VIEWS_FORMS_FORM_PAINT_INFO_H_

#include "evita/gfx/rect_f.h"

namespace gfx {
class Canvas;
}

namespace views {

class FormPaintState;

//////////////////////////////////////////////////////////////////////
//
// FormPaintInfo
//
class FormPaintInfo final {
 public:
  FormPaintInfo(gfx::Canvas* canvas,
                const gfx::RectF& bounds,
                FormPaintState* paint_state);
  ~FormPaintInfo();

  const gfx::RectF bounds() const { return bounds_; }
  gfx::Canvas* canvas() const { return canvas_; }
  FormPaintState* paint_state() const { return paint_state_; }

 private:
  const gfx::RectF bounds_;
  gfx::Canvas* const canvas_;
  FormPaintState* const paint_state_;

  DISALLOW_COPY_AND_ASSIGN(FormPaintInfo);
};

}  // namespace views

#endif  // EVITA_VIEWS_FORMS_FORM_PAINT_INFO_H_
