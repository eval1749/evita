// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_forms_form_window_h)
#define INCLUDE_evita_views_forms_form_window_h

#include "evita/views/window.h"

#include <memory>

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

  private: class FormViewModel;

  private: gfx::Size form_size_;
  private: private: std::unique_ptr<gfx::Canvas> canvas_;
  private: private: const std::unique_ptr<FormViewModel> model_;
  private: gfx::Point offset_;
  private: Window* owner_;
  private: base::string16 title_;

  public: FormWindow(WindowId window_id, dom::Form* form, Window* owner,
                     gfx::Point offset);
  public: FormWindow(WindowId window_id, dom::Form* form);
  public: virtual ~FormWindow();

  private: void DoRealizeWidget();
  private: void TransferFocusIfNeeded();

  // ui::AnimationFrameHandler
  private: virtual void DidBeginAnimationFrame(base::Time time) override;

  // ui::SystemMetricsObserver
  private: virtual void DidChangeIconFont() override;
  private: virtual void DidChangeSystemColor() override;
  private: virtual void DidChangeSystemMetrics() override;

  // ui::Widget
  private: virtual void CreateNativeWindow() const override;
  private: virtual void DidChangeBounds() override;
  private: virtual void DidDestroyWidget() override;
  private: virtual void DidRealize() override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual LRESULT OnMessage(uint32_t message, WPARAM wParam,
                                     LPARAM lParam) override;
  private: virtual void RealizeWidget() override;

  DISALLOW_COPY_AND_ASSIGN(FormWindow);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_forms_form_window_h)
