// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_root_widget_h)
#define INCLUDE_evita_ui_root_widget_h

#include "common/memory/singleton.h"
#include "evita/ui/widget.h"

namespace widgets {

class RootWidget : public Widget,
                   public common::Singleton<RootWidget> {
  DECLARE_CASTABLE_CLASS(RootWidget, Widget);
  public: RootWidget();
  public: ~RootWidget();
  DISALLOW_COPY_AND_ASSIGN(RootWidget);
};

}  // namespace widgets

#endif //!defined(INCLUDE_evita_ui_root_widget_h)
