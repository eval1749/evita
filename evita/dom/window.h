// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_window_h)
#define INCLUDE_evita_dom_window_h

#include "evita/v8_glue/scriptable.h"
#include "evita/widgets/widget_id.h"

namespace dom {

// |Window| is correspond to |Widget|.
class Window : public v8_glue::Scriptable<Window> {
  // Associated wiget id. If associated widget is destroyed, it is changed
  // to |kInvalidWidgetId|.
  private: widgets::WidgetId widget_id_;

  private: Window(widgets::WidgetId widget_id);
  public: virtual ~Window();

  static_assert(sizeof(int) == sizeof(widgets::WidgetId),
                "widgets::WidgetId must be compatible with int.");
  public: int id() const { return widget_id_; }
  public: static v8_glue::WrapperInfo* static_wrapper_info();

  // [G]
  public: virtual gin::ObjectTemplateBuilder
      GetObjectTemplateBuilder(v8::Isolate* isolate) override;

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_window_h)
