// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_table_control_h)
#define INCLUDE_evita_ui_controls_table_control_h

#include "evita/gfx/point_f.h"
#include "evita/ui/base/table_model_observer.h"
#include "evita/ui/widget.h"

#include <memory>
#include <vector>

namespace gfx {
class Canvas;
}

namespace ui {

class KeyEvent;
class MouseEvent;
struct TableColumn;
class TableControlObserver;
class TableModel;

//////////////////////////////////////////////////////////////////////
//
// TableControl
// Implements a table view with header, list of rows and scroll bar for
// scrolling row list.
//
class TableControl final : public ui::Widget {
  DECLARE_CASTABLE_CLASS(TableControl, Widget);

  private: class View;

  private: std::unique_ptr<View> view_;

  public: TableControl(const std::vector<TableColumn>& columns,
                       const TableModel* model,
                       TableControlObserver* observer);
  public: virtual ~TableControl();

  public: TableModelObserver* GetTableModelObserver();
  public: int GetRowState(int row_id) const;
  public: void Select(int row_id);

  // ui::Widget
  private: virtual void DidSetFocus(Widget* last_focused) override;

  DISALLOW_COPY_AND_ASSIGN(TableControl);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_table_control_h)
