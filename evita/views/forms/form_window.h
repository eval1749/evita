// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_forms_form_window_h)
#define INCLUDE_evita_views_forms_form_window_h

#include "evita/views/window.h"

#include <memory>

#include "evita/gc/member.h"
#include "evita/ui/system_metrics_observer.h"

namespace dom {
class Form;
}

namespace gfx {
class Graphics;
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
  private: private: const std::unique_ptr<gfx::Graphics> gfx_;
  private: private: const std::unique_ptr<FormViewModel> model_;
  private: gfx::Rect pending_update_rect_;
  private: base::string16 title_;

  public: FormWindow(views::WindowId window_id, const dom::Form* form);
  public: virtual ~FormWindow();

  public: static bool DoIdle(int hint);
  private: bool OnIdle(int hint);

  // ui::SystemMetricsObserver
  private: virtual void DidChangeIconFont() override;
  private: virtual void DidChangeSystemColor() override;
  private: virtual void DidChangeSystemMetrics() override;

  // ui::Widget
  private: virtual void CreateNativeWindow() const override;
  private: virtual void DidCreateNativeWindow() override;
  private: virtual void DidDestroyWidget() override;
  private: virtual void DidRealize() override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual LRESULT OnMessage(uint32_t message, WPARAM wParam,
                                     LPARAM lParam) override;
  private: virtual void OnPaint(const gfx::Rect paint_rect) override;

  DISALLOW_COPY_AND_ASSIGN(FormWindow);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_forms_form_window_h)
