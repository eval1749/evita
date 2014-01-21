// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_controls_table_control_h)
#define INCLUDE_evita_ui_controls_table_control_h

#include "evita/gfx/point_f.h"
#include "evita/ui/base/table_model_observer.h"
#include "evita/widgets/widget.h"

#include <memory>
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
  private: std::vector<TableColumn> columns_;
  private: std::vector<float> column_widths_;
  private: const TableModel* model_;
  private: std::vector<Row*> rows_;
  private: TableControlObserver* observer_;
  private: float row_height_;
  private: std::unique_ptr<gfx::TextFormat> text_format_;

  public: TableControl(const std::vector<TableColumn>& columns,
                       const TableModel* model,
                       TableControlObserver* observer);
  public: virtual ~TableControl();

  private: void DrawHeaderRow(gfx::Graphics* gfx);
  private: void DrawRow(gfx::Graphics* gfx, gfx::PointF left_top,
                        const Row* row);
  public: bool IsSelected(int model_row) const;
  private: void SchedulePaint();
  public: void Select(int model_row);

  // TableModelObserver
  private: virtual void DidAddItems(int start, int length) override;
  private: virtual void DidChangeItems(int start, int length) override;
  private: virtual void DidRemoveItems(int start, int length) override;

  // Widget
  private: virtual void OnDraw(gfx::Graphics* gfx) override;

  DISALLOW_COPY_AND_ASSIGN(TableControl);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_table_control_h)
