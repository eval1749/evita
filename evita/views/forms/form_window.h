// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_FORMS_FORM_WINDOW_H_
#define EVITA_VIEWS_FORMS_FORM_WINDOW_H_

#include <memory>

#include "evita/views/window.h"

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "evita/dom/public/geometry.h"
#include "evita/gc/member.h"
#include "evita/gfx/canvas_owner.h"
#include "evita/ui/animation/animatable.h"

namespace domapi {
class Form;
}

namespace gfx {
class Canvas;
using common::win::Point;
using common::win::Rect;
using common::win::Size;
}

namespace views {

class FormPaintState;

//////////////////////////////////////////////////////////////////////
//
// FormWindow
//
class FormWindow final : public views::Window, private gfx::CanvasOwner {
  DECLARE_CASTABLE_CLASS(FormWindow, views::Window);

 public:
  FormWindow(WindowId window_id,
             Window* owner,
             const domapi::IntRect& bounds,
             const base::StringPiece16 title);
  ~FormWindow() final;

  void Paint(std::unique_ptr<domapi::Form> form);

 private:
  class FormViewModel;

  // gfx::CanvasOwner
  std::unique_ptr<gfx::SwapChain> CreateSwapChain() final;

  // ui::Widget
  void CreateNativeWindow() const final;
  void DidChangeBounds() final;
  void DidDestroyWidget() final;
  void DidRealize() final;
  void DidRequestDestroy() final;

  std::unique_ptr<gfx::Canvas> canvas_;
  domapi::IntRect form_bounds_;
  Window* const owner_;
  const std::unique_ptr<FormPaintState> paint_state_;
  base::string16 title_;

  DISALLOW_COPY_AND_ASSIGN(FormWindow);
};

}  // namespace views

#endif  // EVITA_VIEWS_FORMS_FORM_WINDOW_H_
