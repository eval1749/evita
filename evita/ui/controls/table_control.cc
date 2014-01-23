// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/controls/table_control.h"

#include <list>
#include <unordered_map>
#include <vector>

#include "common/castable.h"
#include "common/win/scoped_comptr.h"
#include "evita/gfx_base.h"
#include "evita/ui/base/selection_model.h"
#include "evita/ui/base/table_model.h"
#include "evita/ui/controls/table_control_observer.h"

namespace ui {

namespace {

//////////////////////////////////////////////////////////////////////
//
// Item
//
class Item : public common::Castable {
  DECLARE_CASTABLE_CLASS(Item, common::Castable);

  private: gfx::RectF rect_;

  public: Item();
  public: virtual ~Item();

  public: const gfx::RectF& rect() const { return rect_; }
  public: void set_rect(const gfx::RectF& rect) { rect_ = rect; }

  DISALLOW_COPY_AND_ASSIGN(Item);
};

Item::Item() {
}

Item::~Item() {
}

//////////////////////////////////////////////////////////////////////
//
// Column
//
class Column : public Item {
  DECLARE_CASTABLE_CLASS(Column, Item);

  private: TableColumn data_;

  public: Column(const TableColumn& data);
  public: ~Column();

  public: DWRITE_TEXT_ALIGNMENT alignment() const;
  public: int column_id() const { return data_.column_id; }
  public: const base::string16& text() const { return data_.text; }
  public: float width() const { return data_.width; }

  DISALLOW_COPY_AND_ASSIGN(Column);
};

Column::Column(const TableColumn& data)
    : data_(data) {
  auto const kRightPadding = 5;
  data_.width += kRightPadding;
}

Column::~Column() {
}

DWRITE_TEXT_ALIGNMENT Column::alignment() const {
    static const DWRITE_TEXT_ALIGNMENT dwrite_alignment[] = {
      DWRITE_TEXT_ALIGNMENT_LEADING,
      DWRITE_TEXT_ALIGNMENT_TRAILING,
      DWRITE_TEXT_ALIGNMENT_CENTER,
    };
  return dwrite_alignment[static_cast<int>(data_.alignment)];
}

//////////////////////////////////////////////////////////////////////
//
// Row
//
class Row : public Item{
  DECLARE_CASTABLE_CLASS(Row, Item);

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

  DISALLOW_COPY_AND_ASSIGN(Row);
};

Row::Row(int row_id) :
    row_id_(row_id),
    state_(kNone) {
}

Row::~Row() {
}
}

//////////////////////////////////////////////////////////////////////
//
// TableControl::TableControlModel
//
class TableControl::TableControlModel {
  private: std::vector<Column*> columns_;
  private: bool has_focus_;
  private: const TableModel* model_;
  private: gfx::RectF rect_;
  private: std::list<Row*> rows_;
  private: std::unordered_map<int, Row*> row_map_;
  private: float row_height_;
  private: std::unique_ptr<gfx::TextFormat> text_format_;

  public: TableControlModel(const std::vector<TableColumn>& columns,
                            const TableModel* model);
  public: ~TableControlModel();

  public: void DidAddRow(int row_id);
  public: void DidChangeRow(int row_id);
  public: void DidKillFocus();
  public: void DidRemoveRow(int row_id);
  public: void DidResize(const gfx::RectF& rect);
  public: void DidSetFocus();
  public: void Draw(gfx::Graphics* gfx) const;
  private: void DrawHeaderRow(gfx::Graphics* gfx, gfx::PointF left_top) const;
  private: void DrawRow(gfx::Graphics* gfx, const Row* row) const;
  public: bool IsSelected(int row_id) const;
  public: void Select(int row_id);

  DISALLOW_COPY_AND_ASSIGN(TableControlModel);
};

TableControl::TableControlModel::TableControlModel(
    const std::vector<TableColumn>& columns,
    const TableModel* model)
    : has_focus_(false),
      model_(model),
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

TableControl::TableControlModel::~TableControlModel() {
  for (auto row : rows_) {
    delete row;
  }
  for (auto column : columns_) {
    delete column;
  }
}

void TableControl::TableControlModel::DidAddRow(int row_id) {
  auto const row = new Row(row_id);
  rows_.push_back(row);
  row_map_[row_id] = row;
}

void TableControl::TableControlModel::DidChangeRow(int) {
}

void TableControl::TableControlModel::DidKillFocus() {
  has_focus_ = false;
}

void TableControl::TableControlModel::DidRemoveRow(int row_id) {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return;
  }
  auto const row = present->second;
  row_map_.erase(present);
  rows_.remove(row);
  delete row;
}

void TableControl::TableControlModel::DidResize(const gfx::RectF& rect) {
  rect_ = rect;
}

void TableControl::TableControlModel::DidSetFocus() {
  has_focus_ = true;
}

void TableControl::TableControlModel::Draw(gfx::Graphics* gfx) const {
  auto const kLeftMargin = 10.0f;
  auto const kTopMargin = 3.0f;

  gfx::PointF left_top(rect_.left + kLeftMargin, rect_.top + kTopMargin);
  gfx::Brush fill_brush(*gfx, gfx::ColorF::White);

  // Fill top edge
  gfx->FillRectangle(fill_brush, gfx::RectF(
      gfx::PointF(rect_.left, rect_.top),
      gfx::SizeF(rect_.width(), kTopMargin)));

  DrawHeaderRow(gfx, left_top);

  left_top.y += row_height_;

  // Fill between header and rows
  gfx->FillRectangle(fill_brush, gfx::RectF(
      gfx::PointF(rect_.left, left_top.y),
      gfx::SizeF(rect_.width(), kTopMargin)));

  left_top.y += kTopMargin;

  // Rows
  for (auto row : rows_) {
    row->set_rect(gfx::RectF(left_top.x, left_top.y,
                             rect_.right, left_top.y + row_height_));
    DrawRow(gfx, row);
    gfx->Flush();
    left_top.y += row->rect().height();
    if (left_top.y >= rect_.bottom)
      break;
  }

  // Fill right edge
  gfx->FillRectangle(fill_brush, gfx::RectF(
      rect_.left_top(),
      gfx::SizeF(kLeftMargin, left_top.y - rect_.top)));

  // Fill bottom edge
  if (left_top.y < rect_.bottom) {
    gfx->FillRectangle(fill_brush, gfx::RectF(
        gfx::PointF(rect_.left, left_top.y),
        gfx::SizeF(rect_.width(), rect_.bottom - left_top.y)));
  }
}

void TableControl::TableControlModel::DrawHeaderRow(gfx::Graphics* gfx,
    gfx::PointF left_top) const {
  gfx->FillRectangle(gfx::Brush(*gfx, gfx::ColorF::White),
                     gfx::RectF(left_top,
                                gfx::SizeF(static_cast<float>(rect_.width()),
                                           row_height_)));
  gfx::Brush textBrush(*gfx, gfx::ColorF::Black);
  gfx::Brush grayBrush(*gfx, gfx::ColorF::LightGray);
  gfx::PointF cell_left_top(left_top);
  auto column_index = 0u;
  for (auto column : columns_) {
    ++column_index;
    (*text_format_)->SetTextAlignment(column->alignment());
    auto const width = column_index == columns_.size() ?
        rect_.width() - cell_left_top.x : column->width();
    gfx::RectF rect(cell_left_top, gfx::SizeF(width, row_height_));
    column->set_rect(rect);
    auto text = column->text();
    (*gfx)->DrawText(text.data(), text.length(), *text_format_, rect,
                     textBrush);
    gfx->DrawLine(grayBrush, rect.right - 5, rect.top,
                  rect.right - 5, rect.bottom, 0.5f);
    cell_left_top.x += column->width();
  }
}

void TableControl::TableControlModel::DrawRow(gfx::Graphics* gfx,
                                              const Row* row) const {
  auto const bgcolor = row->selected() ?
      has_focus_ ? gfx::ColorF(RGB(51, 153, 255)) :
                    gfx::ColorF(RGB(191, 205, 219)) :
      gfx::ColorF(RGB(255, 255, 255));
  auto const color = row->selected() ?
      has_focus_ ? gfx::ColorF(RGB(255, 255, 255)) :
                    gfx::ColorF(RGB(67, 78, 84)) :
      gfx::ColorF(RGB(0, 0, 0));
  gfx->FillRectangle(gfx::Brush(*gfx, bgcolor), row->rect());
  gfx::Brush textBrush(*gfx, color);
  gfx::PointF cell_left_top(row->rect().left_top());
  auto column_index = 0u;
  for (auto column : columns_) {
    auto const text = model_->GetCellText(row->row_id(), column->column_id());
    (*text_format_)->SetTextAlignment(column->alignment());
    auto const width = column_index == columns_.size() ?
        rect_.width() - cell_left_top.x : column->width();
    gfx::RectF rect(cell_left_top, gfx::SizeF(width, row_height_));
    (*gfx)->DrawText(text.data(), text.length(), *text_format_, rect,
                     textBrush);
    cell_left_top.x += column->width();

  }
}

bool TableControl::TableControlModel::IsSelected(int row_id) const {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return false;
  }
  return present->second->selected();
}

void TableControl::TableControlModel::Select(int row_id) {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return;
  }
  present->second->selected();
}

//////////////////////////////////////////////////////////////////////
//
// TableControl
//
TableControl::TableControl(const std::vector<TableColumn>& columns,
                           const TableModel* model,
                           TableControlObserver* observer)
    : model_(new TableControlModel(columns, model)),
      observer_(observer) {
}

TableControl::~TableControl() {
}

bool TableControl::IsSelected(int row_id) const {
  return model_->IsSelected(row_id);
}

void TableControl::Select(int row_id) {
  model_->Select(row_id);
}

// TableModelObserver
void TableControl::DidAddRow(int row_id) {
  model_->DidAddRow(row_id);
  SchedulePaint();
}

void TableControl::DidChangeRow(int row_id) {
  model_->DidChangeRow(row_id);
  SchedulePaint();
}

void TableControl::DidRemoveRow(int row_id) {
  model_->DidRemoveRow(row_id);
  SchedulePaint();
}

// Widget
void TableControl::DidKillFocus() {
  model_->DidKillFocus();
  SchedulePaint();
}

void TableControl::DidResize() {
  model_->DidResize(gfx::RectF(rect()));
  SchedulePaint();
}

void TableControl::DidSetFocus() {
  model_->DidKillFocus();
  SchedulePaint();
}

void TableControl::OnDraw(gfx::Graphics* gfx) {
  model_->Draw(gfx);
}

}  // namespace ui
