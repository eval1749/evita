// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_WIDGET_H_
#define EVITA_UI_WIDGET_H_

#include <memory>
#include <ostream>

#include "base/macros.h"
#include "base/event_types.h"
#include "base/logging.h"
#include "common/tree/node.h"
#include "common/win/native_window.h"
#include "evita/gfx/rect.h"
#include "evita/gfx/rect_f.h"
#include "evita/ui/compositor/layer_owner.h"
#include "evita/ui/events/event_target.h"
#include "evita/ui/focus_controller.h"

namespace gfx {
class Canvas;
}

namespace ui {

using common::win::NativeWindow;
class FocusController;
class KeyEvent;
class Layer;
class MouseEvent;
class MouseWheelEvent;

//////////////////////////////////////////////////////////////////////
//
// Widget
//
class Widget : public EventTarget,
               public common::tree::Node<Widget>,
               public common::win::MessageDelegate,
               public LayerOwner {
  DECLARE_CASTABLE_CLASS(Widget, EventTarget);

 public:
  ~Widget() override;

  const gfx::Rect& bounds() const { return bounds_; }
  const Widget* container_widget() const;
  Widget* container_widget();

  // |child_window_id()| is used for |NMHDR.idFrom| of |WM_NOTIFY|.
  UINT_PTR child_window_id() const;
  bool has_focus() const;
  bool has_native_focus() const;
  bool is_realized() const { return state_ == kRealized; }
  const gfx::Point origin() const { return bounds_.origin(); }
  bool visible() const { return visible_; }

  // By default a View is owned by its parent unless specified otherwise here.
  void set_owned_by_client() { owned_by_client_ = true; }

  HWND AssociatedHwnd() const;
  void DestroyWidget();
  gfx::RectF GetContentsBounds() const;
  // Returns the bounds of the view in its own coordinates (i.e. position is
  // 0, 0). This function is useful for hit test.
  gfx::Rect GetLocalBounds() const;
  virtual gfx::Size GetPreferredSize() const;
  virtual void Hide();
  gfx::Point MapFromDesktopPoint(const gfx::Point& desktop_point) const;
  gfx::Point MapToDesktopPoint(const gfx::Point& local_point) const;
  // Note: We expose |OnDraw| for real time content resizing during toplevel
  // window resizing.
  virtual void OnDraw(gfx::Canvas* canvas);
  virtual LRESULT OnMessage(uint32_t uMsg, WPARAM wParam, LPARAM lParam);
  virtual void OnPaint(const gfx::Rect rect);
  // Realize widget, top-level widget must be a native window.
  virtual void RealizeWidget();
  void ReleaseCapture();
  virtual void RequestFocus();
  void SetBounds(const gfx::Point& origin, const gfx::Point& bottom_right);
  void SetBounds(const gfx::Point& origin, const gfx::Size& size);
  void SetBounds(const gfx::Rect& rect);
  void SetCapture();
  void SetParentWidget(Widget* new_parent);
  virtual void Show();

 protected:
  explicit Widget(std::unique_ptr<NativeWindow> native_window);
  Widget();

  NativeWindow* native_window() const { return native_window_.get(); }
  virtual void CreateNativeWindow() const;
  virtual void DidActivate();
  virtual void DidAddChildWidget(Widget* new_child);
  virtual void DidChangeBounds();
  virtual void DidChangeChildVisibility(Widget* child);
  virtual void DidChangeHierarchy();
  virtual void DidDestroyWidget();
  // Called on WM_NCDESTORY
  virtual void DidDestroyNativeWindow();
  virtual void DidHide();
  virtual void DidKillFocus(Widget* focused_window);

  virtual void DidRealize();
  virtual void DidRealizeChildWidget(Widget* new_child);
  virtual void DidRemoveChildWidget(Widget* old_child);
  virtual void DidRequestDestroy();
  virtual void DidSetFocus(Widget* last_focused);
  virtual void DidShow();
  virtual HCURSOR GetCursorAt(const gfx::Point& point) const;
  virtual void OnKeyPressed(const KeyEvent& event);
  virtual void OnKeyReleased(const KeyEvent& event);
  virtual void OnMouseEntered(const MouseEvent& event);
  virtual void OnMouseExited(const MouseEvent& event);
  virtual void OnMouseMoved(const MouseEvent& event);
  virtual void OnMousePressed(const MouseEvent& event);
  virtual void OnMouseReleased(const MouseEvent& event);
  virtual void OnMouseWheel(const MouseWheelEvent& event);
  // TODO(eval1749): We should get rid of |ui::Widget::SchedulePaint()|, since
  // it isn't matched with "always paint" model.
  void SchedulePaint();
  void SchedulePaintInRect(const gfx::Rect& rect);
  virtual void WillDestroyWidget();
  virtual void WillDestroyNativeWindow();
  virtual void WillRemoveChildWidget(Widget* old_child);

  // common::win::MessageDelegate
  virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

  // EventTarget
  void OnKeyEvent(KeyEvent* event) override;
  void OnMouseEvent(MouseEvent* event) override;

 private:
  // For |DidKillFocuns()| and |DidSetFocus()|.
  friend class FocusController;

  class HitTestResult;

  enum State {
    kDestroyed = -2,
    kBeingDestroyed = -1,
    kNotRealized,
    kRealized,
  };

  bool has_native_window() const { return static_cast<bool>(native_window_); }

  void DispatchMouseExited();
  void DispatchPaintMessage();
  Widget* GetHostWidget() const;
  LRESULT HandleKeyboardMessage(uint32_t message, WPARAM wParam, LPARAM lParam);
  bool HandleMouseMessage(const base::NativeEvent& native_event);
  HitTestResult HitTest(const gfx::Point& point) const;
  HitTestResult HitTestForMouseEventTarget(const gfx::Point& point) const;
  bool SetCursor();
  void UpdateBounds();

  gfx::Rect bounds_;
  std::unique_ptr<NativeWindow> native_window_;
  // False if this View is owned by its parent - i.e. it will be deleted by its
  // parent during its parents destruction. False is the default.
  bool owned_by_client_;
  State state_;
  bool visible_;

  DISALLOW_COPY_AND_ASSIGN(Widget);
};

}  // namespace ui

std::ostream& operator<<(std::ostream& out, const ui::Widget& widget);
std::ostream& operator<<(std::ostream& out, const ui::Widget* widget);

#endif  // EVITA_UI_WIDGET_H_
