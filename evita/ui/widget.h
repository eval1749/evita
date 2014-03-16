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
#include "common/win/rect.h"

namespace gfx {
class Graphics;
}

namespace ui {

using common::win::NativeWindow;
using common::win::Point;
using common::win::Rect;
class KeyboardEvent;
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

  private: enum State {
    kDestroyed = -2,
    kBeingDestroyed = -1,
    kNotRealized,
    kRealized,
  };

  private: std::unique_ptr<NativeWindow> native_window_;
  private: Rect rect_;
  private: int shown_;
  private: State state_;

  protected: explicit Widget(std::unique_ptr<NativeWindow>&& native_window);
  protected: Widget();
  protected: ~Widget();

  public: Widget& container_widget() const {
    DCHECK(parent_node());
    return *parent_node();
  }

  // |child_window_id()| is used for |NMHDR.idFrom| of |WM_NOTIFY|.
  public: UINT_PTR child_window_id() const;
  public: static bool has_active_focus();
  public: bool has_focus() const;
  public: bool has_native_window() const { 
    return static_cast<bool>(native_window_);
  }
  public: bool is_realized() const { return state_ == kRealized; }
  public: bool is_shown() const { return shown_; }
  protected: NativeWindow* native_window() const {
    return native_window_.get();
  }
  public: const Rect& rect() const { return rect_; }

  // [A]
  public: HWND AssociatedHwnd() const;

  // [C]
  protected: virtual void CreateNativeWindow() const;

  // [D]
  public: void DestroyWidget();
  protected: virtual void DidAddChildWidget(const Widget& widget);
  protected: virtual void DidChangeHierarchy();
  // Called on WM_CREATE
  protected: virtual void DidCreateNativeWindow();
  protected: virtual void DidDestroyWidget();
  // Called on WM_NCDESTORY
  protected: virtual void DidDestroyNativeWindow();
  protected: virtual void DidHide();
  protected: virtual void DidKillFocus();
  protected: virtual void DidRealize();
  protected: virtual void DidRealizeChildWidget(const Widget& widget);
  protected: virtual void DidRemoveChildWidget(const Widget& widget);
  protected: virtual void DidRequestFocus();
  protected: virtual void DidResize();
  protected: virtual void DidShow();
  private: void DispatchPaintMessage();

  // [G]
  public: virtual HCURSOR GetCursorAt(const Point& point) const;
  public: static Widget* GetFocusWidget();
  private: Widget& GetHostWidget() const;
  private: Widget* GetWidgetAt(const Point& point) const;

  // [H]
  public: virtual void Hide();

  // [O]
  // Note: We expose |OnDraw| for real time content resizing during toplevel
  // window resizing.
  public: virtual void OnDraw(gfx::Graphics* gfx);
  protected: virtual void OnKeyPressed(const KeyboardEvent& event);
  protected: virtual void OnKeyReleased(const KeyboardEvent& event);
  public: virtual LRESULT OnMessage(uint32_t uMsg, WPARAM wParam,
                                    LPARAM lParam);
  protected: virtual void OnMouseMoved(const MouseEvent& event);
  protected: virtual void OnMousePressed(const MouseEvent& event);
  protected: virtual void OnMouseReleased(const MouseEvent& event);
  protected: virtual void OnMouseWheel(const MouseWheelEvent& event);
  public: virtual void OnPaint(const Rect rect);
  // TODO(yosi) |ui::Widget::OnScroll| is used for handling |WM_VSCROLL|, Once
  // we implement scroll bar, we should remove this.
  protected: virtual void OnScroll(int request);

  // [R]
  // Realize widget, one of container must be realized with native widnow.
  public: void Realize(const Rect& rect);
  // Realize top-level widget with native window.
  public: void RealizeTopLevelWidget();
  // TODO(yosi) Widget::RealizeWidget() should be pure virtual.
  public: virtual void RealizeWidget();
  public: void ReleaseCapture();
  public: virtual void RequestFocus();
  public: void ResizeTo(const Rect& rect);

  // [S]
  protected: void SchedulePaint();
  protected: void SchedulePaintInRect(const Rect& rect);
  public: void SetCapture();
  private: bool SetCursor();
  public: void SetParentWidget(Widget* new_parent);
  public: virtual void Show();

  // [W]
  protected: virtual void WillDestroyWidget();
  protected: virtual void WillDestroyNativeWindow();
  protected: virtual void WillRemoveChildWidget(const Widget& widget);
  protected: virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam,
                                        LPARAM lParam) override;
  DISALLOW_COPY_AND_ASSIGN(Widget);
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_widget_h)
