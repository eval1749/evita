// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_view_event_handler_h)
#define INCLUDE_evita_dom_view_event_handler_h

#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/strings/string16.h"
#include "evita/dom/window_id.h"

namespace dom {

class ViewEventHandler {
  public: ViewEventHandler() = default;
  public: virtual ~ViewEventHandler() = default;

  public: virtual void DidDestroyWidget(WindowId window_id) = 0;
  public: virtual void DidDropWidget(WindowId source_id,
                                     WindowId target_id)  = 0;
  public: virtual void DidKillFocus(WindowId window_id) = 0;
  public: virtual void DidRealizeWidget(WindowId window_id) = 0;
  public: virtual void DidSetFocus(WindowId window_id) = 0;
  public: virtual void DidStartHost() = 0;
  public: virtual void OpenFile(WindowId window_id,
                                const base::string16& filename) = 0;
  public: virtual void QueryClose(WindowId window_id) = 0;
  public: virtual void RunCallback(base::Closure) = 0;
  public: virtual void WillDestroyHost() = 0;

  DISALLOW_COPY_AND_ASSIGN(ViewEventHandler);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_view_event_handler_h)
