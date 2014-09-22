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

class KeyboardEvent;
class MouseEvent;
struct TableColumn;
class TableControlObserver;
class TableModel;

//////////////////////////////////////////////////////////////////////
//
// TableControl
//
class TableControl final : public ui::Widget, public TableModelObserver {
  DECLARE_CASTABLE_CLASS(TableControl, Widget);

  private: class View;

  private: std::unique_ptr<View> view_;
  private: TableControlObserver* observer_;

  public: TableControl(const std::vector<TableColumn>& columns,
                       const TableModel* model,
                       TableControlObserver* observer);
  public: virtual ~TableControl();

  public: int GetRowState(int row_id) const;
  public: void RenderIfNeeded(gfx::Canvas* canvas);
  public: void Select(int row_id);

  // ui::TableModelObserver
  private: virtual void DidAddRow(int row_id) override;
  private: virtual void DidChangeRow(int row_id) override;
  private: virtual void DidRemoveRow(int row_id) override;

  // ui::Widget
  private: virtual void DidKillFocus(ui::Widget* focused_window) override;
  private: virtual void DidRealize() override;
  private: virtual void DidChangeBounds() override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual void DidShow() override;
  private: virtual void OnKeyPressed(const KeyboardEvent& event) override;
  private: virtual void OnMouseExited(const ui::MouseEvent& event) override;
  private: virtual void OnMouseMoved(const ui::MouseEvent& event) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;

  DISALLOW_COPY_AND_ASSIGN(TableControl);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_table_control_h)
