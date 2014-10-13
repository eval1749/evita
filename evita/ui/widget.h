// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_widget_h)
#define INCLUDE_evita_ui_widget_h

#include <memory>

#include "base/basictypes.h"
#include "base/event_types.h"
#include "base/logging.h"
#include "common/castable.h"
#include "common/tree/node.h"
#include "common/win/native_window.h"
#include "evita/gfx/rect.h"
#include "evita/gfx/rect_f.h"
#include "evita/ui/compositor/layer_owner.h"
#include "evita/ui/focus_controller.h"

namespace gfx {
class Canvas;
}

namespace ui {

using common::win::NativeWindow;
class FocusController;
class KeyboardEvent;
class Layer;
class MouseEvent;
class MouseWheelEvent;

//////////////////////////////////////////////////////////////////////
//
// Widget
//
class Widget : public common::Castable,
               public common::tree::Node<Widget>,
               public common::win::MessageDelegate,
               public LayerOwner {
  DECLARE_CASTABLE_CLASS(Widget, Castable);

  // For |DidKillFocuns()| and |DidSetFocus()|.
  friend class FocusController;

  private: class HitTestResult;

  private: enum State {
    kDestroyed = -2,
    kBeingDestroyed = -1,
    kNotRealized,
    kRealized,
  };

  private: gfx::Rect bounds_;
  private: std::unique_ptr<NativeWindow> native_window_;
  // False if this View is owned by its parent - i.e. it will be deleted by its
  // parent during its parents destruction. False is the default.
  private: bool owned_by_client_;
  private: State state_;
  private: bool visible_;

  protected: explicit Widget(std::unique_ptr<NativeWindow> native_window);
  protected: Widget();
  protected: ~Widget();

  public: const gfx::Rect& bounds() const { return bounds_; }
  public: const Widget* container_widget() const;
  public: Widget* container_widget();

  // |child_window_id()| is used for |NMHDR.idFrom| of |WM_NOTIFY|.
  public: UINT_PTR child_window_id() const;
  public: bool has_focus() const;
  public: bool has_native_focus() const;
  private: bool has_native_window() const {
    return static_cast<bool>(native_window_);
  }
  public: bool is_realized() const { return state_ == kRealized; }
  protected: NativeWindow* native_window() const {
    return native_window_.get();
  }
  public: const gfx::Point origin() const { return bounds_.origin(); }
  public: bool visible() const { return visible_; }

  // By default a View is owned by its parent unless specified otherwise here.
  public: void set_owned_by_client() { owned_by_client_ = true; }

  // [A]
  public: HWND AssociatedHwnd() const;

  // [C]
  protected: virtual void CreateNativeWindow() const;

  // [D]
  public: void DestroyWidget();
  protected: virtual void DidAddChildWidget(Widget* new_child);
  protected: virtual void DidChangeBounds();
  protected: virtual void DidChangeChildVisibility(Widget* child);
  protected: virtual void DidChangeHierarchy();
  protected: virtual void DidDestroyWidget();
  // Called on WM_NCDESTORY
  protected: virtual void DidDestroyNativeWindow();
  protected: virtual void DidHide();
  protected: virtual void DidKillFocus(Widget* focused_window);

  protected: virtual void DidRealize();
  protected: virtual void DidRealizeChildWidget(Widget* new_child);
  protected: virtual void DidRemoveChildWidget(Widget* old_child);
  protected: virtual void DidRequestDestroy();
  protected: virtual void DidSetFocus(Widget* last_focused);
  protected: virtual void DidShow();
  private: void DispatchMouseExited();
  private: void DispatchPaintMessage();

  // [G]
  public: gfx::RectF GetContentsBounds() const;
  protected: virtual HCURSOR GetCursorAt(const gfx::Point& point) const;
  private: Widget* GetHostWidget() const;
  // Returns the bounds of the view in its own coordinates (i.e. position is
  // 0, 0). This function is useful for hit test.
  public: gfx::Rect GetLocalBounds() const;
  public: virtual gfx::Size GetPreferredSize() const;

  // [H]
  private: LRESULT HandleKeyboardMessage(uint32_t message, WPARAM wParam,
                                         LPARAM lParam);
  private: bool HandleMouseMessage(const base::NativeEvent& native_event);
  public: virtual void Hide();
  private: HitTestResult HitTest(const gfx::Point& point) const;
  private: HitTestResult HitTestForMouseEventTarget(
      const gfx::Point& point) const;

  // [M]
  public: gfx::Point MapFromDesktopPoint(const gfx::Point& desktop_point) const;
  public: gfx::Point MapToDesktopPoint(const gfx::Point& local_point) const;

  // [O]
  // Note: We expose |OnDraw| for real time content resizing during toplevel
  // window resizing.
  public: virtual void OnDraw(gfx::Canvas* canvas);
  protected: virtual void OnKeyPressed(const KeyboardEvent& event);
  protected: virtual void OnKeyReleased(const KeyboardEvent& event);
  public: virtual LRESULT OnMessage(uint32_t uMsg, WPARAM wParam,
                                    LPARAM lParam);
  protected: virtual void OnMouseExited(const MouseEvent& event);
  protected: virtual void OnMouseMoved(const MouseEvent& event);
  protected: virtual void OnMousePressed(const MouseEvent& event);
  protected: virtual void OnMouseReleased(const MouseEvent& event);
  protected: virtual void OnMouseWheel(const MouseWheelEvent& event);
  public: virtual void OnPaint(const gfx::Rect rect);

  // [R]
  // Realize widget, top-level widget must be a native window.
  public: virtual void RealizeWidget();
  public: void ReleaseCapture();
  public: virtual void RequestFocus();

  // [S]
  // TODO(eval1749) We should get rid of |ui::Widget::SchedulePaint()|, since
  // it isn't matched with "always paint" model.
  protected: void SchedulePaint();
  protected: void SchedulePaintInRect(const gfx::Rect& rect);
  public: void SetBounds(const gfx::Point& origin,
                         const gfx::Point& bottom_right);
  public: void SetBounds(const gfx::Point& origin, const gfx::Size& size);
  public: void SetBounds(const gfx::Rect& rect);
  public: void SetCapture();
  private: bool SetCursor();
  public: void SetParentWidget(Widget* new_parent);
  public: virtual void Show();

  // [U]
  private: void UpdateBounds();

  // [W]
  protected: virtual void WillDestroyWidget();
  protected: virtual void WillDestroyNativeWindow();
  protected: virtual void WillRemoveChildWidget(Widget* old_child);
  protected: virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam,
                                        LPARAM lParam) override;
  DISALLOW_COPY_AND_ASSIGN(Widget);
};

} // namespace ui

#include <ostream>

std::ostream& operator<<(std::ostream& out, const ui::Widget& widget);
std::ostream& operator<<(std::ostream& out, const ui::Widget* widget);

#endif //!defined(INCLUDE_evita_ui_widget_h)
