// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_window_h)
#define INCLUDE_evita_dom_window_h

#include <vector>

#include "evita/v8_glue/scriptable.h"
#include "evita/dom/widget_id.h"

namespace dom {

// |Window| is correspond to |Widget| in DOM world.
class Window : public v8_glue::Scriptable<Window> {
  private: class WidgetIdMapper;
  friend class WidgetIdMapper;

  public: enum State {
    kDestroyed = -1,
    kNotRealized,
    kRealizing,
    kRealized,
  };

  private: std::vector<Window*> child_windows_;
  private: Window* parent_window_;
  private: State state_;
  // Associated wiget id. If associated widget is destroyed, it is changed
  // to |kInvalidWidgetId|.
  private: WidgetId widget_id_;

  protected: Window();
  public: virtual ~Window();

  public: WidgetId id() const { return widget_id_; }
  public: v8_glue::Nullable<Window> parent_window() const {
    return parent_window_;
  }
  public: State state() const { return state_; }
  public: static v8_glue::WrapperInfo* static_wrapper_info();
  public: WidgetId widget_id() const { return widget_id_; }

  public: void AddWindow(Window* window);
  public: static void DidDestroyWidget(WidgetId widget_id);
  public: static void DidRealizeWidget(WidgetId widget_id);
  public: static Window* FromWidgetId(WidgetId widget_id);
  public: bool IsDescendantOf(Window* other) const;
  public: void Realize();
  public: void RemoveWindow(Window* window);
  public: static void ResetForTesting();

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace dom

#include <ostream>

std::ostream& operator<<(std::ostream& ostream, const dom::Window& window);
std::ostream& operator<<(std::ostream& ostream, const dom::Window* window);

#endif //!defined(INCLUDE_evita_dom_window_h)
