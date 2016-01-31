// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/forms/form_window.h"

#include <unordered_map>

#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "common/win/win32_verify.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/swap_chain.h"
#include "evita/dom/public/form.h"
#include "evita/dom/public/form_controls.h"
#include "evita/views/forms/form_painter.h"
#include "evita/views/forms/form_paint_info.h"
#include "evita/views/forms/form_paint_state.h"

namespace views {

//////////////////////////////////////////////////////////////////////
//
// FormWindow
//
FormWindow::FormWindow(WindowId window_id,
                       Window* owner,
                       const domapi::IntRect& bounds,
                       base::StringPiece16 title)
    : views::Window(ui::NativeWindow::Create(this), window_id),
      form_bounds_(bounds),
      owner_(owner),
      paint_state_(new FormPaintState()),
      title_(title.as_string()) {}

FormWindow::~FormWindow() {}

void FormWindow::Paint(std::unique_ptr<domapi::Form> form) {
  TRACE_EVENT0("view", "FormWindow::Paint");
  if (form_bounds_.size() != form->bounds().size()) {
    form_bounds_ = form->bounds();
    RECT window_rect;
    ::GetWindowRect(AssociatedHwnd(), &window_rect);
    window_rect.right = window_rect.left + form_bounds_.width();
    window_rect.bottom = window_rect.top + form_bounds_.height();
    const auto extended_window_style =
        static_cast<DWORD>(::GetWindowLong(AssociatedHwnd(), GWL_EXSTYLE));
    const auto window_style =
        static_cast<DWORD>(::GetWindowLong(AssociatedHwnd(), GWL_STYLE));
    const auto has_menu = false;
    WIN32_VERIFY(::AdjustWindowRectEx(&window_rect, window_style, has_menu,
                                      extended_window_style));
    SetBounds(gfx::Rect(window_rect));
  }

  if (title_ != form->title()) {
    title_ = form->title();
    ::SetWindowTextW(AssociatedHwnd(), title_.c_str());
  }

  if (!visible() || !canvas_->IsReady())
    return;

  FormPaintInfo paint_info(canvas_.get(), GetContentsBounds(),
                           paint_state_.get());
  FormPainter().Paint(paint_info, *form);
  paint_state_->Update(std::move(form));
}

// gfx::CanvasOwner
std::unique_ptr<gfx::SwapChain> FormWindow::CreateSwapChain() {
  return gfx::SwapChain::CreateForHwnd(AssociatedHwnd());
}

// ui::Animatable
bool FormWindow::CanHandleAnimationFrame() const {
  return canvas_ && canvas_->IsReady();
}

void FormWindow::DidBeginAnimationFrame(const base::TimeTicks& now) {
  // Nothing to do
}

// ui::Widget
void FormWindow::CreateNativeWindow() const {
  struct Local {
    static gfx::Rect GetDefaultBounds(const gfx::Size& form_size) {
      return gfx::Rect(gfx::Point(CW_USEDEFAULT, CW_USEDEFAULT), form_size);
    }

    // Compute form window bounds as center of foreground window if possible.
    static gfx::Rect ComputeFormWindowBounds(uint32_t extended_window_style,
                                             uint32_t window_style,
                                             const gfx::Size& form_size) {
      const auto foreground_hwnd = ::GetForegroundWindow();
      if (!foreground_hwnd)
        return GetDefaultBounds(form_size);
      RECT active_window_rect;
      if (!::GetWindowRect(foreground_hwnd, &active_window_rect))
        return GetDefaultBounds(form_size);
      gfx::Rect active_window_bounds(active_window_rect);
      auto window_bounds =
          gfx::Rect(active_window_bounds.origin() +
                        ((active_window_bounds.size() - form_size) / 2),
                    form_size);
      const auto has_menu = false;
      RECT window_rect(window_bounds);
      WIN32_VERIFY(::AdjustWindowRectEx(&window_rect, window_style, has_menu,
                                        extended_window_style));
      return gfx::Rect(window_rect);
    }
  };

  const auto& form_origin = gfx::Point(form_bounds_.x(), form_bounds_.y());
  const auto& form_size =
      gfx::Size(form_bounds_.width(), form_bounds_.height());

  if (title_.empty()) {
    // Popup window
    DCHECK(owner_);
    const auto extended_window_style = WS_EX_LAYERED | WS_EX_TOOLWINDOW;
    const auto window_style = WS_POPUPWINDOW | WS_VISIBLE;
    const auto& screen_point = owner_->MapToDesktopPoint(form_origin);
    native_window()->CreateWindowEx(extended_window_style, window_style,
                                    L"popup", owner_->AssociatedHwnd(),
                                    screen_point, form_size);
    return;
  }

  // Place dialog window at center of active window.
  const auto extended_window_style = WS_EX_LAYERED | WS_EX_TOOLWINDOW;
  const auto window_style = WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE;
  const auto form_window_bounds = Local::ComputeFormWindowBounds(
      extended_window_style, window_style, form_size);
  native_window()->CreateWindowEx(
      extended_window_style, window_style, title_.c_str(), nullptr,
      form_window_bounds.origin(), form_window_bounds.size());
}

void FormWindow::DidChangeBounds() {
  canvas_->SetBounds(GetContentsBounds());
  SchedulePaint();
  Window::DidChangeBounds();
}

void FormWindow::DidDestroyWidget() {
  Window::DidDestroyWidget();
}

void FormWindow::DidRealize() {
  // TODO(eval1749): We should get default value of form window transparency
  // from CSS.
  ::SetLayeredWindowAttributes(*native_window(), RGB(0, 0, 0), 80 * 255 / 100,
                               LWA_ALPHA);
  canvas_.reset(new gfx::Canvas(this));
  Window::DidRealize();
}

void FormWindow::DidRequestDestroy() {
  // TODO(eval1749): Should we dispatch "close" event to JavaScript?
  Hide();
}

}  // namespace views
