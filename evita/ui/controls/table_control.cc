// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/controls/table_control.h"

#include "common/win/scoped_comptr.h"
#include "evita/gfx_base.h"
#include "evita/ui/controls/table_control_observer.h"
#include "evita/ui/controls/table_model.h"

namespace ui {

namespace {

const DWRITE_TEXT_ALIGNMENT dwrite_alignment[] = {
  WRITE_TEXT_ALIGNMENT_LEADING,
  DWRITE_TEXT_ALIGNMENT_TRAILING,
  DWRITE_TEXT_ALIGNMENT_CENTER,
};

}  // namespace

TableControl::TableControl(const std::vector<TableColumn> columns,
                           TableModel* model, TableControlObserver* observer)
    : columns_(columns),
      model_(model),
      observer_(observer),
      row_height_(15.0f),
      text_format_(new gfx::TextFormat(L"MS Shell Dlg 2", row_height - 4)) {
  auto const num_rows =model->row_count();
  for (auto row = 0; row < num_rows; ++row) {
    model_to_view_.push_back(row);
    view_to_model_.push_back(row);
  }
  column_widths_.resize(columns_.size());
  auto widths = column_widths_;
  for (auto column : columns_) {
    *widths = column.width;
    ++widths;
  }
}

TableControl::~TableControl() {
}

void TableControl::PainHeaderRow(gfx::Graphics* gfx) {
  gfx->FillRectangle(gfx::Brush(gfx::White), rect().left_top(),
                     gfx::SizeF(rect().width(), row_height_));
  gfx::Brush textBrush(gfx::Black);
  gfx::PointF cell_left_top(row_left_top);
  auto column_width_runner = column_widths_.begin();
  for (auto column : columns_) {
    text_format_->SetAlignment(dwrite_alignment[column.alignment]);
    gfx->DrawText(text_format_, textBrush,
                  gfx::Rect(cell_left_top,
                            gfx::SizeF(column.width, row_height_)),
                  column.text, column.text.length());
    cell_left_top.x += column.width;
  }
}

void TableControl::PaintRow(gfx::Graphics* gfx, gfx::PointF row_left_top,
                            int row) {
  gfx->FillRectangle(gfx::Brush(gfx::White), row_left_top,
                     gfx::SizeF(rect().width(), rect.height()));
  gfx::Brush textBrush(gfx::Black);
  gfx::PointF cell_left_top(row_left_top);
  auto column_width_runner = column_widths_.begin();
  for (auto column : columns_) {
    gfx::RectF cell_rect(cell_left_top,
                         gfx::SizeF(*column_width_runner, row_height_));
    auto text = model_->text(row, column->id);
    text_format_->SetAlignment(dwrite_alignment[column.alignment]);
    gfx->DrawText(text_format_, textBrush, cell_rect, text, text.length());
    cell_left_top.y += row_height_;
    ++column_width_runner;
  }

  gfx->Flush();
}

// Widget
void TableControl::OnPaint(gfx::Graphics* gfx) {
  gfx::Graphics::DrawingScope drawing_scope(*gfx);

  PainHeaderRow();

  gfx::PointF left_top(rect.left(), rect.top() + row_height_);
  for (auto model_index : view_to_model_) {
    auto row = model_->row(model_index);
    PaintRow(left_top, row);
    left_top.y += row_height_;
    if (left_top.y >= rect().bottom)
      break;
  }

  gfx::Brush fill_brush(gfx::White);
  gfx->FillRect(fill_brush, gfx::RectF(gfx::PointF(rect().left(), left_top)),
                                       gfx::SizeF(rect.size()));
}

}  // namespace ui
