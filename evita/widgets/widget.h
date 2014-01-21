// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_widgets_widget_h)
#define INCLUDE_widgets_widget_h

#include <ostream>
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

namespace widgets {

typedef common::win::NativeWindow NativeWindow;
typedef common::win::Point Point;
typedef common::win::Rect Rect;

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
  public: void Destroy();
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
  protected: virtual void DidResize();
  protected: virtual void DidSetFocus();
  protected: virtual void DidShow();
  private: void DispatchPaintMessage();

  // [G]
  public: virtual const char* GetClass() const { return "Widget"; }
  public: virtual HCURSOR GetCursorAt(const Point& point) const;
  public: static Widget* GetFocusWidget();
  private: Widget& GetHostWidget() const;
  private: Widget* GetWidgetAt(const Point& point) const;

  // [H]
  public: virtual void Hide();

  // [O]
  protected: virtual void OnDraw(gfx::Graphics* gfx);
  public: virtual bool OnIdle(uint32_t idle_count);
  public: virtual void OnLeftButtonDown(uint32_t flags, const Point& point);
  public: virtual void OnLeftButtonUp(uint32_t flags, const Point& point);
  public: virtual LRESULT OnMessage(uint32_t uMsg, WPARAM wParam,
                                    LPARAM lParam);
  public: virtual void OnMouseMove(uint32_t flags, const Point& point);
  protected: virtual LRESULT OnNotify(NMHDR* nmhdr);
  public: virtual void OnPaint(const Rect rect);

  // [R]
  // Realize widget, one of container must be realized with native widnow.
  public: void Realize(const Rect& rect);
  // Realize top-level widget with native window.
  public: void RealizeTopLevelWidget();
  // TODO(yosi) Widget::RealizeWidget() should be pure virtual.
  public: virtual void RealizeWidget();
  public: void ReleaseCapture();
  public: void ResizeTo(const Rect& rect);

  // [S]
  public: void SetCapture();
  private: bool SetCursor();
  public: virtual void SetFocus();
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

#define DEBUG_WIDGET_PRINTF(mp_format, ...) \
  DEBUG_PRINTF(DEBUG_WIDGET_FORMAT " " mp_format, \
    DEBUG_WIDGET_ARG(this), __VA_ARGS__)

#define DEBUG_WIDGET_FORMAT "%s@%p"
#define DEBUG_WIDGET_ARG(mp_widget) \
  ((mp_widget) ? (mp_widget)->class_name() : "null"), (mp_widget)

} // namespace widgets

std::ostream& operator<<(std::ostream& out, const widgets::Widget& widget);
std::ostream& operator<<(std::ostream& out, const widgets::Widget* widget);

#define DVLOG_WIDGET(n) \
    DVLOG(n) << __FUNCTION__ << " " << *this << \
        " hwnd=" << AssociatedHwnd() << " "

#endif //!defined(INCLUDE_widgets_widget_h)
