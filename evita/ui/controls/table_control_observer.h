// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_controls_table_control_observer_h)
#define INCLUDE_evita_ui_controls_table_control_observer_h

#include <vector>

namespace ui {

class TableControlObserver;

class TableControlObserver {
  public: TableControlObserver();
  public: virtual TableControlObserver();
  public: virtual void OnKeyDown(int key_code);
  public: virtual void OnSelectionChanged();
};

}  // namespace widgets

#endif //!defined(INCLUDE_evita_ui_controls_table_control_observer_h)
