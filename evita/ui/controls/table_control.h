// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_CONTROLS_TABLE_CONTROL_H_
#define EVITA_UI_CONTROLS_TABLE_CONTROL_H_

#include <memory>
#include <vector>

#include "evita/gfx/point_f.h"
#include "evita/ui/base/table_model_observer.h"
#include "evita/ui/widget.h"

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

 public:
  TableControl(const std::vector<TableColumn>& columns,
               const TableModel* model,
               TableControlObserver* observer);
  ~TableControl() final;

  TableModelObserver* GetTableModelObserver();
  int GetRowState(int row_id) const;
  void Select(int row_id);

 private:
  class View;

  // ui::Widget
  void DidSetFocus(Widget* last_focused) final;

  std::unique_ptr<View> view_;

  DISALLOW_COPY_AND_ASSIGN(TableControl);
};

}  // namespace ui

#endif  // EVITA_UI_CONTROLS_TABLE_CONTROL_H_
