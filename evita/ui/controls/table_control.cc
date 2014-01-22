// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/controls/table_control.h"

#include "common/win/scoped_comptr.h"
#include "evita/gfx_base.h"
#include "evita/ui/base/selection_model.h"
#include "evita/ui/base/table_model.h"
#include "evita/ui/controls/table_control_observer.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// TableControl::Column
//
class TableControl::Column {
  private: TableColumn data_;

  public: Column(const TableColumn& data);
  public: ~Column();

  public: DWRITE_TEXT_ALIGNMENT alignment() const;
  public: int column_id() const { return data_.column_id; }
  public: const base::string16& text() const { return data_.text; }
  public: float width() const { return data_.width; }
};

TableControl::Column::Column(const TableColumn& data)
    : data_(data) {
}

TableControl::Column::~Column() {
}

DWRITE_TEXT_ALIGNMENT TableControl::Column::alignment() const {
    static const DWRITE_TEXT_ALIGNMENT dwrite_alignment[] = {
      DWRITE_TEXT_ALIGNMENT_LEADING,
      DWRITE_TEXT_ALIGNMENT_TRAILING,
      DWRITE_TEXT_ALIGNMENT_CENTER,
    };
  return dwrite_alignment[static_cast<int>(data_.alignment)];
}

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

  private: int row_id_;
  private: int state_;

  public: Row(int row_id);
  public: ~Row();

  public: int row_id() const { return row_id_; }
  public: bool selected() const { return state_ & kSelected; }

  public: void Select() { state_ |= kSelected; }
};

TableControl::Row::Row(int row_id) :
    row_id_(row_id),
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
    : model_(model),
      observer_(observer),
      row_height_(17.0f),
      text_format_(new gfx::TextFormat(L"MS Shell Dlg 2", row_height_ - 4)) {
  auto const num_rows = static_cast<size_t>(model->GetRowCount());
  for (auto index = 0u; index < num_rows; ++index) {
    auto const row_id = model_->GetRowId(static_cast<int>(index));
    auto const row = new Row(row_id);
    rows_.push_back(row);
    row_map_[row_id] = row;
  }
  columns_.resize(columns.size());
  for (auto index = 0u; index < columns.size(); ++index) {
    columns_[index] = new Column(columns[index]);
  }
}

TableControl::~TableControl() {
  for (auto row : rows_) {
    delete row;
  }
  for (auto column : columns_) {
    delete column;
  }
}

void TableControl::DrawHeaderRow(gfx::Graphics* gfx) {
  gfx->FillRectangle(gfx::Brush(*gfx, gfx::ColorF::White),
                     gfx::RectF(rect().left_top(),
                                gfx::SizeF(static_cast<float>(rect().width()),
                                           row_height_)));
  gfx::Brush textBrush(*gfx, gfx::ColorF::Black);
  gfx::PointF cell_left_top(rect().left_top());
  for (auto column : columns_) {
    (*text_format_)->SetTextAlignment(column->alignment());
    gfx::RectF rect(cell_left_top, gfx::SizeF(column->width(), row_height_));
    auto text = column->text();
    (*gfx)->DrawText(text.data(), text.length(), *text_format_, rect,
                     textBrush);
    cell_left_top.x += column->width();
  }
}

void TableControl::DrawRow(gfx::Graphics* gfx, gfx::PointF row_left_top,
                           const Row* row) {
  auto const bgcolor = row->selected() ?
      has_focus() ? gfx::ColorF(RGB(51, 153, 255)) :
                    gfx::ColorF(RGB(191, 205, 219)) :
      gfx::ColorF(RGB(255, 255, 255));
  auto const color = row->selected() ?
      has_focus() ? gfx::ColorF(RGB(255, 255, 255)) :
                    gfx::ColorF(RGB(67, 78, 84)) :
      gfx::ColorF(RGB(0, 0, 0));
  gfx->FillRectangle(gfx::Brush(*gfx, bgcolor),
                     gfx::RectF(row_left_top,
                                gfx::SizeF(static_cast<float>(rect().width()),
                                           row_height_)));
  gfx::Brush textBrush(*gfx, color);
  gfx::PointF cell_left_top(row_left_top);
  for (auto column : columns_) {
    auto const text = model_->GetCellText(row->row_id(), column->column_id());
    (*text_format_)->SetTextAlignment(column->alignment());
    gfx::RectF rect(cell_left_top, gfx::SizeF(column->width(), row_height_));
    (*gfx)->DrawText(text.data(), text.length(), *text_format_, rect,
                     textBrush);
    cell_left_top.x += column->width();
  }

  gfx->Flush();
}

bool TableControl::IsSelected(int row_id) const {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return false;
  }
  return present->second->selected();
}

void TableControl::SchedulePaint() {
  ::InvalidateRect(AssociatedHwnd(), &rect(), true);
}

void TableControl::Select(int row_id) {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return;
  }
  present->second->selected();
}


// TableModelObserver
void TableControl::DidAddRow(int row_id) {
  auto const row = new Row(row_id);
  rows_.push_back(row);
  row_map_[row_id] = row;
  SchedulePaint();
}

void TableControl::DidChangeRow(int) {
  SchedulePaint();
}

void TableControl::DidRemoveRow(int row_id) {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return;
  }
  auto const row = present->second;
  row_map_.erase(present);
  rows_.remove(row);
  delete row;
  SchedulePaint();
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
