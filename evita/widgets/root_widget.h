// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_widgets_root_widget_h)
#define INCLUDE_widgets_root_widget_h

#include "base/memory/singleton.h"
#include "evita/widgets/container_widget.h"

namespace widgets {

class RootWidget : public ContainerWidget,
                   public base::Singleton<RootWidget> {
  DECLARE_CASTABLE_CLASS(RootWidget, ContainerWidget);
  public: RootWidget();
  public: ~RootWidget();
  DISALLOW_COPY_AND_ASSIGN(RootWidget);
};

}  // namespace widgets

#endif //!defined(INCLUDE_widgets_root_widget_h)
