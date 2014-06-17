// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_controls_table_control_h)
#define INCLUDE_evita_ui_controls_table_control_h

#include "evita/gfx/point_f.h"
#include "evita/ui/base/table_model_observer.h"
#include "evita/ui/widget.h"

#include <memory>
#include <vector>

namespace gfx {
class Canvas;
using common::win::Point;
class TextFormat;
}

namespace ui {

class KeyboardEvent;
class MouseEvent;
struct TableColumn;
class TableControlObserver;
class TableModel;

class TableControl :
    public ui::Widget,
    public TableModelObserver {

  DECLARE_CASTABLE_CLASS(TableControl, Widget);

  private: class TableControlModel;

  private: std::unique_ptr<TableControlModel> model_;
  private: TableControlObserver* observer_;

  public: TableControl(const std::vector<TableColumn>& columns,
                       const TableModel* model,
                       TableControlObserver* observer);
  public: virtual ~TableControl();

  public: int GetRowState(int row_id) const;
  public: void Select(int row_id);
  private: void UpdateViewIfNeeded();

  // TableModelObserver
  private: virtual void DidAddRow(int row_id) override;
  private: virtual void DidChangeRow(int row_id) override;
  private: virtual void DidRemoveRow(int row_id) override;

  // Widget
  private: virtual void DidKillFocus(ui::Widget* focused_window) override;
  private: virtual void DidRealize() override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual void DidShow() override;
  private: virtual void OnDraw(gfx::Canvas* gfx) override;
  private: virtual void OnKeyPressed(const KeyboardEvent& event) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;

  DISALLOW_COPY_AND_ASSIGN(TableControl);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_table_control_h)
