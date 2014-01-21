// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_controls_table_control_h)
#define INCLUDE_evita_ui_controls_table_control_h

#include "evita/widgets/widget.h"

#include <memory>
#include <vector>

namespace gfx {
class Graphics;
class TextFormat;
}

namespace ui {

class TableControlObserver;

class TableControl : public widgets::Widget {
  private: std::vector<TableColumn> columns_;
  private: std::vector<int> column_widths_;
  private: const TableModel* model_;
  private: std::vector<int> model_to_view_;
  private: TableControlObserver* observer_;
  private: float row_height_;
  private: std::unique_ptr<gfx::TextFormat> text_format_;
  private: std::vector<int> view_to_model_;

  public: TableControl(const std::vector<TableControl> columns,
                       const TableModel* model,
                       TableControlObserver* observer);
  public: virtual ~TableControl();

  private: void PainHeaderRow(gfx::Graphics* gfx);
  private: void PaintRow(gfx::Graphics* gfx, gfx::PointF left_top, int row);
  public: void Select(int model_row);

  // Widget
  private: virtual void OnPaint(gfx::Graphics* gfx) override;
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_table_control_h)
