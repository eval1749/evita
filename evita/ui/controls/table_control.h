// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_controls_table_control_h)
#define INCLUDE_evita_ui_controls_table_control_h

#include "evita/gfx/point_f.h"
#include "evita/ui/base/table_model_observer.h"
#include "evita/widgets/widget.h"

#include <memory>
#include <list>
#include <unordered_map>
#include <vector>

namespace gfx {
class Graphics;
class TextFormat;
}

namespace ui {

struct TableColumn;
class TableControlObserver;
class TableModel;

class TableControl :
    public widgets::Widget,
    public TableModelObserver {
  private: class Row;
  private: class Column;
  private: std::vector<Column*> columns_;
  private: const TableModel* model_;
  private: std::list<Row*> rows_;
  private: std::unordered_map<int, Row*> row_map_;
  private: TableControlObserver* observer_;
  private: float row_height_;
  private: std::unique_ptr<gfx::TextFormat> text_format_;

  public: TableControl(const std::vector<TableColumn>& columns,
                       const TableModel* model,
                       TableControlObserver* observer);
  public: virtual ~TableControl();

  private: void DrawHeaderRow(gfx::Graphics* gfx, gfx::PointF left_top);
  private: void DrawRow(gfx::Graphics* gfx, gfx::PointF left_top,
                        const Row* row);
  public: bool IsSelected(int row_id) const;
  public: void Select(int row_id);

  // TableModelObserver
  private: virtual void DidAddRow(int row_id) override;
  private: virtual void DidChangeRow(int row_id) override;
  private: virtual void DidRemoveRow(int row_id) override;

  // Widget
  private: virtual void OnDraw(gfx::Graphics* gfx) override;

  DISALLOW_COPY_AND_ASSIGN(TableControl);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_table_control_h)
