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
  auto const kRightPadding = 5;
  data_.width += kRightPadding;
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
      row_height_(20.0f),
      text_format_(new gfx::TextFormat(L"MS Shell Dlg 2", row_height_ - 6)) {

  {
    common::ComPtr<IDWriteInlineObject> inline_object;
    COM_VERIFY(gfx::FactorySet::instance()->dwrite().
        CreateEllipsisTrimmingSign(*text_format_.get(), &inline_object));
    DWRITE_TRIMMING trimming {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
    (*text_format_.get())->SetTrimming(&trimming, inline_object);
  }

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

void TableControl::DrawHeaderRow(gfx::Graphics* gfx, gfx::PointF left_top) {
  gfx->FillRectangle(gfx::Brush(*gfx, gfx::ColorF::White),
                     gfx::RectF(left_top,
                                gfx::SizeF(static_cast<float>(rect().width()),
                                           row_height_)));
  gfx::Brush textBrush(*gfx, gfx::ColorF::Black);
  gfx::Brush grayBrush(*gfx, gfx::ColorF::LightGray);
  gfx::PointF cell_left_top(left_top);
  auto column_index = 0u;
  for (auto column : columns_) {
    ++column_index;
    (*text_format_)->SetTextAlignment(column->alignment());
    auto const width = column_index == columns_.size() ?
        rect().width() - cell_left_top.x : column->width();
    gfx::RectF rect(cell_left_top, gfx::SizeF(width, row_height_));
    auto text = column->text();
    (*gfx)->DrawText(text.data(), text.length(), *text_format_, rect,
                     textBrush);
    gfx->DrawLine(grayBrush, rect.right - 5, rect.top,
                  rect.right - 5, rect.bottom, 0.5f);
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
  auto column_index = 0u;
  for (auto column : columns_) {
    auto const text = model_->GetCellText(row->row_id(), column->column_id());
    (*text_format_)->SetTextAlignment(column->alignment());
    auto const width = column_index == columns_.size() ?
        rect().width() - cell_left_top.x : column->width();
    gfx::RectF rect(cell_left_top, gfx::SizeF(width, row_height_));
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
  auto const kLeftMargin = 10.0f;
  auto const kTopMargin = 3.0f;

  gfx::RectF rectf(rect());
  gfx::PointF left_top(rectf.left + kLeftMargin, rectf.top + kTopMargin);
  gfx::Brush fill_brush(*gfx, gfx::ColorF::White);

  // Fill top edge
  gfx->FillRectangle(fill_brush, gfx::RectF(
      gfx::PointF(rectf.left, rectf.top),
      gfx::SizeF(rectf.width(), kTopMargin)));

  DrawHeaderRow(gfx, left_top);

  left_top.y += row_height_;

  // Fill between header and rows
  gfx->FillRectangle(fill_brush, gfx::RectF(
      gfx::PointF(rectf.left, left_top.y),
      gfx::SizeF(rectf.width(), kTopMargin)));

  left_top.y += kTopMargin;

  // Rows
  for (auto row : rows_) {
    DrawRow(gfx, left_top, row);
    left_top.y += row_height_;
    if (left_top.y >= rectf.bottom)
      break;
  }

  // Fill right edge
  gfx->FillRectangle(fill_brush, gfx::RectF(
      rectf.left_top(),
      gfx::SizeF(kLeftMargin, left_top.y - rectf.top)));

  // Fill bottom edge
  if (left_top.y < rectf.bottom) {
    gfx->FillRectangle(fill_brush, gfx::RectF(
        gfx::PointF(rectf.left, left_top.y),
        gfx::SizeF(rectf.width(), rectf.bottom - left_top.y)));
  }
}

}  // namespace ui
