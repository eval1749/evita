// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/controls/table_control.h"

#include "common/win/scoped_comptr.h"
#include "evita/gfx_base.h"
#include "evita/ui/base/selection_model.h"
#include "evita/ui/base/table_model.h"
#include "evita/ui/controls/table_control_observer.h"

namespace ui {

namespace {

const DWRITE_TEXT_ALIGNMENT dwrite_alignment[] = {
  DWRITE_TEXT_ALIGNMENT_LEADING,
  DWRITE_TEXT_ALIGNMENT_TRAILING,
  DWRITE_TEXT_ALIGNMENT_CENTER,
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TableControl::Row
//
class TableControl::Row {
  private: enum RowState {
    kNone = 0,
    kSelected = 1,
    kFocused = 1 << 1,
  };

  private: int model_index_;
  private: int state_;

  public: Row(int model_index);
  public: ~Row();

  public: int index() const { return model_index_; }

  public: bool IsSelected() const { return state_ & kSelected; }
  public: void Select() { state_ |= kSelected; }
};

TableControl::Row::Row(int model_index) :
    model_index_(model_index),
    state_(kNone) {
}

TableControl::Row::~Row() {
}

//////////////////////////////////////////////////////////////////////
//
// TableControl
//
TableControl::TableControl(const std::vector<TableColumn>& columns,
                           const TableModel* model,
                           TableControlObserver* observer)
    : columns_(columns),
      model_(model),
      observer_(observer),
      row_height_(15.0f),
      text_format_(new gfx::TextFormat(L"MS Shell Dlg 2", row_height_ - 4)) {
  auto const num_rows = static_cast<size_t>(model->GetRowCount());
  rows_.resize(num_rows);
  for (auto row_index = 0u; row_index < num_rows; ++row_index) {
    rows_[row_index] = new Row(static_cast<int>(row_index));
  }
  column_widths_.resize(columns_.size());
  auto widths = column_widths_.begin();
  for (auto column : columns_) {
    *widths = column.width;
    ++widths;
  }
}

TableControl::~TableControl() {
  for (auto row : rows_) {
    delete row;
  }
}

void TableControl::DrawHeaderRow(gfx::Graphics* gfx) {
  gfx->FillRectangle(gfx::Brush(*gfx, gfx::ColorF::White),
                     gfx::RectF(rect().left_top(),
                                gfx::SizeF(static_cast<float>(rect().width()),
                                           row_height_)));
  gfx::Brush textBrush(*gfx, gfx::ColorF::Black);
  gfx::PointF cell_left_top(rect().left_top());
  auto column_width_runner = column_widths_.begin();
  for (auto column : columns_) {
    (*text_format_)->SetTextAlignment(dwrite_alignment[
        static_cast<int>(column.alignment)]);
    gfx::RectF rect(cell_left_top, gfx::SizeF(column.width, row_height_));
    (*gfx)->DrawText(column.text.data(), column.text.length(), *text_format_,
                     rect,  textBrush);
    cell_left_top.x += column.width;
  }
}

void TableControl::DrawRow(gfx::Graphics* gfx, gfx::PointF row_left_top,
                           const Row* row) {
  auto const bgcolor = row->IsSelected() ?
      has_focus() ? gfx::ColorF(RGB(51, 153, 255)) :
                    gfx::ColorF(RGB(191, 205, 219)) :
      gfx::ColorF(RGB(255, 255, 255));
  auto const color = row->IsSelected() ?
      has_focus() ? gfx::ColorF(RGB(255, 255, 255)) :
                    gfx::ColorF(RGB(67, 78, 84)) :
      gfx::ColorF(RGB(0, 0, 0));
  gfx->FillRectangle(gfx::Brush(*gfx, bgcolor),
                     gfx::RectF(row_left_top,
                                gfx::SizeF(static_cast<float>(rect().width()),
                                           row_height_)));
  gfx::Brush textBrush(*gfx, color);
  gfx::PointF cell_left_top(row_left_top);
  auto column_width_runner = column_widths_.begin();
  for (auto column : columns_) {
    gfx::RectF cell_rect(cell_left_top,
                         gfx::SizeF(*column_width_runner, row_height_));
    auto text = model_->GetCellText(row->index(), column.id);
    (*text_format_)->SetTextAlignment(dwrite_alignment[
        static_cast<int>(column.alignment)]);
    gfx::RectF rect(cell_left_top, gfx::SizeF(column.width, row_height_));
    (*gfx)->DrawText(text.data(), text.length(), *text_format_, rect,
                     textBrush);
    cell_left_top.y += row_height_;
    ++column_width_runner;
  }

  gfx->Flush();
}

void TableControl::SchedulePaint() {
  ::InvalidateRect(AssociatedHwnd(), &rect(), true);
}

// TableModelObserver
void TableControl::DidAddItems(int start, int length) {
  DCHECK_EQ(rows_.size(), static_cast<size_t>(start));
  auto const offset = rows_.size();
  rows_.resize(static_cast<size_t>(rows_.size() + length));
  for (int i = 0; i < length; ++i) {
    rows_[offset + i] = new Row(start + i);
  }
  SchedulePaint();
}

void TableControl::DidChangeItems(int, int) {
  SchedulePaint();
}

void TableControl::DidRemoveItems(int start, int length) {
  auto const begin = rows_.begin() + start;
  auto const end = rows_.begin() + start + length;
  for (auto it = begin; it != begin; ++it) {
    delete *it;
  }
  rows_.erase(begin, end);
}

// Widget
void TableControl::OnDraw(gfx::Graphics* gfx) {
  gfx::Graphics::DrawingScope drawing_scope(*gfx);

  DrawHeaderRow(gfx);

  gfx::PointF left_top(static_cast<float>(rect().left),
                       static_cast<float>(rect().top) + row_height_);
  for (auto row : rows_) {
    DrawRow(gfx, left_top, row);
    left_top.y += row_height_;
    if (left_top.y >= rect().bottom)
      break;
  }

  if (left_top.y < rect().bottom) {
    gfx::Brush fill_brush(*gfx, gfx::ColorF::White);
    gfx->FillRectangle(fill_brush,
                       gfx::RectF(left_top, gfx::SizeF(
                          static_cast<float>(rect().width()),
                          static_cast<float>(rect().bottom) - left_top.y)));
  }
}

}  // namespace ui
