// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_view_event_handler_h)
#define INCLUDE_evita_dom_view_event_handler_h

#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/strings/string16.h"
#include "evita/dom/widget_id.h"

namespace dom {

class ViewEventHandler {
  public: ViewEventHandler() = default;
  public: virtual ~ViewEventHandler() = default;

  public: virtual void DidStartHost() = 0;
  public: virtual void DidDestroyWidget(WidgetId widget_id) = 0;
  public: virtual void DidRealizeWidget(WidgetId widget_id) = 0;
  public: virtual void RunCallback(base::Closure) = 0;
  public: virtual void WillDestroyHost() = 0;

  DISALLOW_COPY_AND_ASSIGN(ViewEventHandler);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_view_event_handler_h)
