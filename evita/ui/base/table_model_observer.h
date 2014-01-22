// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_base_table_model_observer_h)
#define INCLUDE_evita_ui_base_table_model_observer_h

#include "base/strings/string16.h"

namespace ui {

class TableModelObserver {
  public: TableModelObserver();
  public: virtual ~TableModelObserver();

  public: virtual void DidAddRow(int row_id) = 0;
  public: virtual void DidChangeRow(int row_id) = 0;
  public: virtual void DidRemoveRow(int row_id) = 0;

  DISALLOW_COPY_AND_ASSIGN(TableModelObserver);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_base_table_model_observer_h)
