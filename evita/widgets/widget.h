// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_widgets_widget_h)
#define INCLUDE_widgets_widget_h

#include "base/castable.h"
#include "base/tree/node.h"
#include "base/win/native_window.h"
#include "base/win/rect.h"
#include <memory>

namespace widgets {

class ContainerWidget;
typedef base::win::NativeWindow NativeWindow;
typedef base::win::Point Point;
typedef base::win::Rect Rect;

//////////////////////////////////////////////////////////////////////
//
// Widget
//
class Widget
    : public base::tree::Node_<Widget, ContainerWidget,
                               std::unique_ptr<NativeWindow>&&>,
      public base::win::MessageDelegate,
      public base::Castable {
  DECLARE_CASTABLE_CLASS(Widget, Castable);

  // Life time of native_window_ ends at WM_NCDESTROY rather than
  // destruction of Widget.
  private: std::unique_ptr<NativeWindow> native_window_;
  private: bool realized_;
  private: Rect rect_;
  private: int shown_;

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
  public: bool is_realized() const { return realized_; }
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

#include "base/logging.h"
namespace logging {
base::string16 ToString16(const widgets::Widget&);
}
#endif //!defined(INCLUDE_widgets_widget_h)
