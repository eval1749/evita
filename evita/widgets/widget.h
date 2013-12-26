// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_widgets_widget_h)
#define INCLUDE_widgets_widget_h

#include <ostream>
#include <memory>

#include "common/castable.h"
#include "common/tree/node.h"
#include "common/win/native_window.h"
#include "common/win/rect.h"

namespace widgets {

class ContainerWidget;
typedef common::win::NativeWindow NativeWindow;
typedef common::win::Point Point;
typedef common::win::Rect Rect;

//////////////////////////////////////////////////////////////////////
//
// Widget
//
class Widget
    : public common::tree::Node_<Widget, ContainerWidget,
                               std::unique_ptr<NativeWindow>&&>,
      public common::win::MessageDelegate,
      public common::Castable {
  DECLARE_CASTABLE_CLASS(Widget, Castable);

  private: enum State {
    kDestroyed = -2,
    kBeingDestroyed = -1,
    kNotRealized,
    kRealized,
  };

  // Life time of native_window_ ends at WM_NCDESTROY rather than
  // destruction of Widget.
  private: std::unique_ptr<NativeWindow> native_window_;
  private: Rect rect_;
  private: int shown_;
  private: State state_;

  protected: explicit Widget(
      std::unique_ptr<NativeWindow>&& native_window);
  protected: Widget();
  protected: ~Widget();

  public: ContainerWidget& container_widget() const {
    ASSERT(parent_node());
    return *parent_node();
  }
  public: bool has_focus() const;
  public: bool has_native_window() const { 
    return static_cast<bool>(native_window_);
  }
  public: virtual bool is_container() const { return false; }
  public: bool is_realized() const { return state_ == kRealized; }
  public: bool is_shown() const { return shown_; }
  // Expose |is_top_level()| for iterator.
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
  public: virtual void DidChangeHierarchy();
  // Called on WM_CREATE
  protected: virtual void DidCreateNativeWindow();
  protected: virtual void DidDestroyWidget();
  // Called on WM_NCDESTORY
  protected: virtual void DidDestroyNativeWindow();
  public: virtual void DidHide() {}
  public: virtual void DidKillFocus() {}
  public: virtual void DidRealize() {}
  public: virtual void DidResize() {}
  public: virtual void DidSetFocus() {}
  public: virtual void DidShow() {}

  // [G]
  public: virtual const char* GetClass() const { return "Widget"; }
  public: virtual HCURSOR GetCursorAt(const Point& point) const;

  // [H]
  public: virtual void Hide();

  // [O]
  public: virtual bool OnIdle(uint idle_count);
  public: virtual void OnLeftButtonDown(uint flags, const Point& point);
  public: virtual void OnLeftButtonUp(uint flags, const Point& point);
  public: virtual LRESULT OnMessage(uint uMsg, WPARAM wParam, LPARAM lParam);
  public: virtual void OnMouseMove(uint flags, const Point& point);
  public: virtual void OnPaint(const Rect rect);

  // [R]
  // Realize widget, one of container must be realized with native widnow.
  public: void Realize(const Rect& rect);

  // Realize top-level widget with native window.
  public: void RealizeTopLevelWidget();
  public: void ReleaseCapture() const;
  public: void ResizeTo(const Rect& rect);

  // [S]
  public: void SetCapture() const;
  public: virtual void SetFocus();
  public: void SetParentWidget(const ContainerWidget& new_parent);
  public: virtual void Show();

  // [T]
  public: virtual const ContainerWidget* ToContainer() const {
    return nullptr;
  }
  public: virtual ContainerWidget* ToContainer() { return nullptr; }

  // [W]
  public: virtual void WillDestroyWidget();
  public: virtual void WillDestroyNativeWindow();
  public: virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam,
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

#endif //!defined(INCLUDE_widgets_widget_h)
