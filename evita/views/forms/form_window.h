// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_FORMS_FORM_WINDOW_H_
#define EVITA_VIEWS_FORMS_FORM_WINDOW_H_

#include <memory>

#include "evita/views/window.h"

#include "evita/gc/member.h"
#include "evita/ui/animation/animatable.h"
#include "evita/ui/system_metrics_observer.h"

namespace dom {
class Form;
}

namespace gfx {
class Canvas;
using common::win::Point;
using common::win::Rect;
using common::win::Size;
}

namespace ui {
class Control;
}

namespace views {

//////////////////////////////////////////////////////////////////////
//
// FormWindow
//
class FormWindow final : public views::Window,
                         private ui::SystemMetricsObserver {
  DECLARE_CASTABLE_CLASS(FormWindow, views::Window);

 public:
  FormWindow(WindowId window_id,
             dom::Form* form,
             Window* owner,
             gfx::Point offset);
  FormWindow(WindowId window_id, dom::Form* form);
  ~FormWindow() final;

 private:
  class FormViewModel;

  void DoRealizeWidget();

  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(base::Time time) final;

  // ui::SystemMetricsObserver
  void DidChangeIconFont() final;
  void DidChangeSystemColor() final;
  void DidChangeSystemMetrics() final;

  // ui::Widget
  void CreateNativeWindow() const final;
  void DidChangeBounds() final;
  void DidDestroyWidget() final;
  void DidRealize() final;
  void DidRequestDestroy() final;
  void RealizeWidget() final;

  gfx::Size form_size_;
  std::unique_ptr<gfx::Canvas> canvas_;
  const std::unique_ptr<FormViewModel> model_;
  gfx::Point offset_;
  Window* owner_;
  base::string16 title_;

  DISALLOW_COPY_AND_ASSIGN(FormWindow);
};

}  // namespace views

#endif  // EVITA_VIEWS_FORMS_FORM_WINDOW_H_
