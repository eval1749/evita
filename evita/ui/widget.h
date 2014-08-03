// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_widget_h)
#define INCLUDE_evita_ui_widget_h

#include <memory>

#include "base/basictypes.h"
#include "base/logging.h"
#include "common/castable.h"
#include "common/tree/node.h"
#include "common/win/native_window.h"
#include "evita/gfx/rect.h"
#include "evita/gfx/rect_f.h"

namespace gfx {
class Canvas;
}

namespace ui {

using common::win::NativeWindow;
using common::win::Point;
using common::win::Rect;
using common::win::Size;
class KeyboardEvent;
class Layer;
class MouseEvent;
class MouseWheelEvent;

//////////////////////////////////////////////////////////////////////
//
// Widget
//
class Widget
    : public common::Castable,
      public common::tree::Node<Widget>,
      public common::win::MessageDelegate {
  DECLARE_CASTABLE_CLASS(Widget, Castable);

  private: class HitTestResult;

  private: enum State {
    kDestroyed = -2,
    kBeingDestroyed = -1,
    kNotRealized,
    kRealized,
  };

  private: Rect bounds_;
  private: std::unique_ptr<Layer> layer_;
  private: std::unique_ptr<NativeWindow> native_window_;
  private: int shown_;
  private: State state_;

  protected: explicit Widget(std::unique_ptr<NativeWindow>&& native_window);
  protected: Widget();
  protected: ~Widget();

  public: const Rect& bounds() const { return bounds_; }
  public: Widget& container_widget() const {
    DCHECK(parent_node());
    return *parent_node();
  }

  // |child_window_id()| is used for |NMHDR.idFrom| of |WM_NOTIFY|.
  public: UINT_PTR child_window_id() const;
  public: static bool has_active_focus();
  public: bool has_focus() const;
  public: bool has_native_focus() const;
  public: bool has_native_window() const { 
    return static_cast<bool>(native_window_);
  }
  public: bool is_realized() const { return state_ == kRealized; }
  public: Layer* layer() const { return layer_.get(); }
  protected: NativeWindow* native_window() const {
    return native_window_.get();
  }
  public: bool visible() const { return shown_; }

  // [A]
  public: HWND AssociatedHwnd() const;

  // [C]
  protected: virtual void CreateNativeWindow() const;

  // [D]
  public: void DestroyWidget();
  protected: virtual void DidAddChildWidget(const Widget& widget);
  protected: virtual void DidChangeBounds();
  protected: virtual void DidChangeChildVisibility(Widget* child);
  protected: virtual void DidChangeHierarchy();
  protected: virtual void DidDestroyWidget();
  // Called on WM_NCDESTORY
  protected: virtual void DidDestroyNativeWindow();
  protected: virtual void DidHide();
  protected: virtual void DidKillFocus(ui::Widget* focused_window);

  // Called when widget, which has native window, loses native focus. This is
  // good time to restore caret background image.
  protected: virtual void DidKillNativeFocus();
  protected: virtual void DidRealize();
  protected: virtual void DidRealizeChildWidget(const Widget& widget);
  protected: virtual void DidRemoveChildWidget(const Widget& widget);
  protected: virtual void DidSetFocus(ui::Widget* last_focused);

  // Called when widget, which has native window, get native focus. This is
  // good time to prepare caret rendering.
  protected: virtual void DidSetNativeFocus();
  protected: virtual void DidShow();
  private: void DispatchMouseExited();
  private: void DispatchPaintMessage();

  // [G]
  public: gfx::RectF GetContentsBounds() const;
  protected: virtual HCURSOR GetCursorAt(const Point& point) const;
  public: static Widget* GetFocusWidget();
  private: Widget& GetHostWidget() const;
  public: virtual gfx::Size GetPreferredSize() const;

  // [H]
  private: LRESULT HandleKeyboardMessage(uint32_t message, WPARAM wParam,
                                         LPARAM lParam);
  private: void HandleMouseMessage(uint32_t message, WPARAM wParam,
                                   LPARAM lParam);
  public: virtual void Hide();
  private: HitTestResult HitTest(const Point& point) const;
  private: HitTestResult HitTestForMouseEventTarget(const Point& point) const;

  // [M]
  public: Point MapFromDesktopPoint(const Point& desktop_point) const;
  public: Point MapToDesktopPoint(const Point& local_point) const;

  // [O]
  // Note: We expose |OnDraw| for real time content resizing during toplevel
  // window resizing.
  public: virtual void OnDraw(gfx::Canvas* gfx);
  protected: virtual void OnKeyPressed(const KeyboardEvent& event);
  protected: virtual void OnKeyReleased(const KeyboardEvent& event);
  public: virtual LRESULT OnMessage(uint32_t uMsg, WPARAM wParam,
                                    LPARAM lParam);
  protected: virtual void OnMouseExited(const MouseEvent& event);
  protected: virtual void OnMouseMoved(const MouseEvent& event);
  protected: virtual void OnMousePressed(const MouseEvent& event);
  protected: virtual void OnMouseReleased(const MouseEvent& event);
  protected: virtual void OnMouseWheel(const MouseWheelEvent& event);
  public: virtual void OnPaint(const Rect rect);
  // TODO(yosi) |ui::Widget::OnScroll| is used for handling |WM_VSCROLL|, Once
  // we implement scroll bar, we should remove this.
  protected: virtual void OnScroll(int request);

  // [R]
  // Realize widget, one of container must be realized with native window.
  public: void Realize(const Rect& rect);
  // TODO(yosi) Widget::RealizeWidget() should be pure virtual.
  public: virtual void RealizeWidget();
  public: void ReleaseCapture();
  public: virtual void RequestFocus();

  // [S]
  protected: void SchedulePaint();
  protected: void SchedulePaintInRect(const Rect& rect);
  public: void SetBounds(const Rect& rect);
  public: void SetCapture();
  private: bool SetCursor();
  public: void SetLayer(Layer* layer);
  public: void SetParentWidget(Widget* new_parent);
  public: virtual void Show();

  // [U]
  private: void UpdateBounds();

  // [W]
  protected: virtual void WillDestroyWidget();
  protected: virtual void WillDestroyNativeWindow();
  protected: virtual void WillRemoveChildWidget(const Widget& widget);
  protected: virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam,
                                        LPARAM lParam) override;
  DISALLOW_COPY_AND_ASSIGN(Widget);
};

} // namespace ui

#include <ostream>

std::ostream& operator<<(std::ostream& out, const ui::Widget& widget);
std::ostream& operator<<(std::ostream& out, const ui::Widget* widget);

#define DVLOG_WIDGET(n) \
    DVLOG(n) << __FUNCTION__ << " " << *this << \
        " hwnd=" << AssociatedHwnd() << " "

#endif //!defined(INCLUDE_evita_ui_widget_h)
