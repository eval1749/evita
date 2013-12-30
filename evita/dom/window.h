// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_window_h)
#define INCLUDE_evita_dom_window_h

#include <vector>

#include "evita/v8_glue/scriptable.h"
#include "evita/widgets/widget_id.h"

static_assert(sizeof(int) == sizeof(widgets::WidgetId),
                "WidgetId must be compatible with int.");

namespace dom {

// |Window| is correspond to |Widget| in DOM world.
class Window : public v8_glue::Scriptable<Window> {
  private: class WidgetIdMapper;
  friend class WidgetIdMapper;

  protected: typedef widgets::WidgetId WidgetId;

  private: std::vector<Window*> child_windows_;
  private: Window* parent_window_;
  // Associated wiget id. If associated widget is destroyed, it is changed
  // to |kInvalidWidgetId|.
  private: WidgetId widget_id_;

  protected: Window();
  public: virtual ~Window();

  public: int id() const { return widget_id_; }
  public: Window* parent_window() const { return parent_window_; }
  public: static v8_glue::WrapperInfo* static_wrapper_info();
  public: WidgetId widget_id() const { return widget_id_; }

  public: void AddWindow(Window* window);
  public: static void DidDestroyWidget(WidgetId widget_id);
  public: static Window* FromWidgetId(WidgetId widget_id);
  public: bool IsDescendantOf(Window* other) const;
  public: void RemoveWindow(Window* window);

  public: virtual gin::ObjectTemplateBuilder
      GetObjectTemplateBuilder(v8::Isolate* isolate) override;

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace dom

#include <ostream>

std::ostream& operator<<(std::ostream& ostream, const dom::Window& window);
std::ostream& operator<<(std::ostream& ostream, const dom::Window* window);

#endif //!defined(INCLUDE_evita_dom_window_h)
