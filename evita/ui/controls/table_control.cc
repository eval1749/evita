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
#include "evita/ui/events/event.h"

namespace ui {

namespace {

gfx::ColorF RgbToColorF(int red, int green, int blue) {
  return gfx::ColorF(static_cast<float>(red) / 255,
                     static_cast<float>(green) / 255,
                     static_cast<float>(blue) / 255);
}

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

  public: Item* HitTest(const gfx::PointF& point) const;

  DISALLOW_COPY_AND_ASSIGN(Item);
};

Item::Item() {
}

Item::~Item() {
}

Item* Item::HitTest(const gfx::PointF& point) const {
  return rect_.Contains(point) ? const_cast<Item*>(this) : nullptr;
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

  public: enum RowState {
    kNone = 0,
    kFocused = 1,
    kSelected = 1 << 1,
    kCut = 1 << 2,
    kDropHilited = 1 << 3,
    Glow = 1 << 4,
    Activating = 1 << 5,
  };

  private: int row_id_;
  private: int state_;

  public: Row(int row_id);
  public: ~Row();

  public: int row_id() const { return row_id_; }
  public: bool selected() const { return state_ & kSelected; }
  public: int state() const { return state_; }

  public: void Select() { state_ |= kSelected; }
  public: void UpdateState(int new_state, int state_mask);

  DISALLOW_COPY_AND_ASSIGN(Row);
};

Row::Row(int row_id) :
    row_id_(row_id),
    state_(kNone) {
}

Row::~Row() {
}

void Row::UpdateState(int new_state, int state_mask) {
  state_ &= ~state_mask;
  state_ |= new_state;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TableControl::TableControlModel
//
class TableControl::TableControlModel {
  private: std::vector<Column*> columns_;
  private: gfx::RectF dirty_rect_;
  private: bool has_focus_;
  private: const TableModel* model_;
  private: gfx::RectF rect_;
  private: std::list<Row*> rows_;
  private: std::unordered_map<int, Row*> row_map_;
  private: float row_height_;
  private: SelectionModel selection_;
  private: std::unique_ptr<gfx::TextFormat> text_format_;

  public: TableControlModel(const std::vector<TableColumn>& columns,
                            const TableModel* model);
  public: ~TableControlModel();

  public: void DidAddRow(int row_id);
  public: void DidChangeRow(int row_id);
  public: void DidKillFocus();
  public: void DidRemoveRow(int row_id);
  public: void DidResize(const gfx::RectF& rect);
  public: void DidRequestFocus();
  public: void Draw(gfx::Graphics* gfx) const;
  private: void DrawHeaderRow(gfx::Graphics* gfx, gfx::PointF left_top) const;
  private: void DrawRow(gfx::Graphics* gfx, const Row* row) const;
  public: void ExtendSelection(int direction);
  private: const Row* GetRowById(int row_id) const;
  private: int GetRowIndex(const Row* row) const;
  public: int GetRowState(int row_id) const;
  private: Item* HitTest(const gfx::PointF& point) const;
  public: void MakeSelectionViewDirty();
  public: void MoveSelection(int direction);
  public: void OnMousePressed(const ui::MouseEvent& event);
  public: gfx::RectF ResetDirtyRect();
  public: void Select(int row_id);
  private: void UpdateSelectionView();

  DISALLOW_COPY_AND_ASSIGN(TableControlModel);
};

TableControl::TableControlModel::TableControlModel(
    const std::vector<TableColumn>& columns,
    const TableModel* model)
    : has_focus_(false),
      model_(model),
      row_height_(20.0f),
      selection_(model->GetRowCount()),
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
  selection_.DidAddItem();
}

void TableControl::TableControlModel::DidChangeRow(int row_id) {
  auto row = GetRowById(row_id);
  if (!row)
    return;
  dirty_rect_.Unite(row->rect());
}

void TableControl::TableControlModel::DidKillFocus() {
  has_focus_ = false;
  MakeSelectionViewDirty();
}

void TableControl::TableControlModel::DidRemoveRow(int row_id) {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return;
  }
  auto row = present->second;
  auto index = GetRowIndex(row);
  if (index >= 0)
    selection_.DidRemoveItem(index);
  row_map_.erase(present);
  rows_.remove(row);
  delete row;
  UpdateSelectionView();
}

void TableControl::TableControlModel::DidResize(const gfx::RectF& rect) {
  rect_ = rect;
  dirty_rect_ = rect_;
}

void TableControl::TableControlModel::DidRequestFocus() {
  has_focus_ = true;
  if (selection_.empty() && rows_.size()) {
    selection_.CollapseTo(0);
    UpdateSelectionView();
    return;
  }
  MakeSelectionViewDirty();
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
    (*gfx)->DrawText(text.data(), static_cast<uint32_t>(text.length()),
                     *text_format_, rect, textBrush);
    gfx->DrawLine(grayBrush, rect.right - 5, rect.top,
                  rect.right - 5, rect.bottom, 0.5f);
    cell_left_top.x += column->width();
  }
}

void TableControl::TableControlModel::DrawRow(gfx::Graphics* gfx,
                                              const Row* row) const {
  auto const bgcolor = row->selected() ?
      has_focus_ ? RgbToColorF(51, 153, 255) : RgbToColorF(191, 205, 219) :
      gfx::ColorF(gfx::ColorF::White);
  auto const color = row->selected() ?
      has_focus_ ? gfx::ColorF(gfx::ColorF::White) : RgbToColorF(67, 78, 84) :
      gfx::ColorF(gfx::ColorF::Black);
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
    (*gfx)->DrawText(text.data(), static_cast<uint32_t>(text.length()),
                     *text_format_, rect, textBrush);
    cell_left_top.x += column->width();

  }
}

void TableControl::TableControlModel::ExtendSelection(int direction) {
  selection_.Extend(direction);
  UpdateSelectionView();
}

const Row* TableControl::TableControlModel::GetRowById(int row_id) const {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return nullptr;
  }
  return present->second;
}

int TableControl::TableControlModel::GetRowIndex(const Row* present) const {
  auto index = 0;
  for (auto row : rows_) {
    if (row == present)
      return index;
    ++index;
  }
  return -1;
}

int TableControl::TableControlModel::GetRowState(int row_id) const {
  auto row = GetRowById(row_id);
  return row ? row->state() : 0;
}

Item* TableControl::TableControlModel::HitTest(
    const gfx::PointF& point) const {
  for (auto column : columns_) {
    if (auto item = column->HitTest(point))
      return item;
  }

  for (auto row : rows_) {
    if (auto item = row->HitTest(point))
      return item;
  }

  return nullptr;
}

void TableControl::TableControlModel::MakeSelectionViewDirty() {
  auto index = 0;
  for(auto row : rows_) {
    if (selection_.IsSelected(index)) {
      row->UpdateState(row->selected() ? Row::kSelected : 0, Row::kSelected);
      dirty_rect_.Unite(row->rect());
    }
    ++index;
  }
}

void TableControl::TableControlModel::MoveSelection(int direction) {
  selection_.Move(direction);
  UpdateSelectionView();
}

void TableControl::TableControlModel::OnMousePressed(
    const ui::MouseEvent& event) {
  auto const item = HitTest(event.location());
  if (!item)
   return;
  auto row = item->as<Row>();
  if (!row)
    return;
  auto index = GetRowIndex(row);
  if (index < 0)
    return;
  if (event.control_key())
    selection_.Add(index);
  else if (event.shift_key())
    selection_.ExtendTo(index);
  else
    selection_.CollapseTo(index);
  UpdateSelectionView();
}

gfx::RectF TableControl::TableControlModel::ResetDirtyRect() {
  auto dirty_rect = dirty_rect_;
  dirty_rect_ = gfx::RectF();
  return dirty_rect;
}

void TableControl::TableControlModel::Select(int row_id) {
  auto row = GetRowById(row_id);
  if (!row)
    return;
  auto index = GetRowIndex(row);
  if (index < 0)
    return;
  selection_.CollapseTo(index);
  UpdateSelectionView();
}

void TableControl::TableControlModel::UpdateSelectionView() {
  auto index = 0;
  for(auto row : rows_) {
    if (row->selected() != selection_.IsSelected(index)) {
      row->UpdateState(row->selected() ? 0 : Row::kSelected, Row::kSelected);
      dirty_rect_.Unite(row->rect());
    }
    ++index;
  }
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

int TableControl::GetRowState(int row_id) const {
  return model_->GetRowState(row_id);
}

void TableControl::Select(int row_id) {
  model_->Select(row_id);
}

void TableControl::UpdateViewIfNeeded() {
  auto dirty_rect = model_->ResetDirtyRect();
  if (!is_shown() || dirty_rect.empty())
    return;
  SchedulePaintInRect(gfx::Rect(dirty_rect));
}

// TableModelObserver
void TableControl::DidAddRow(int row_id) {
  model_->DidAddRow(row_id);
  UpdateViewIfNeeded();
}

void TableControl::DidChangeRow(int row_id) {
  model_->DidChangeRow(row_id);
  UpdateViewIfNeeded();
}

void TableControl::DidRemoveRow(int row_id) {
  model_->DidRemoveRow(row_id);
  UpdateViewIfNeeded();
}

// Widget
void TableControl::DidKillFocus() {
  model_->DidKillFocus();
  UpdateViewIfNeeded();
}

void TableControl::DidRealize() {
  model_->DidResize(gfx::RectF(rect()));
  UpdateViewIfNeeded();
}

void TableControl::DidRequestFocus() {
  model_->DidRequestFocus();
  UpdateViewIfNeeded();
}

void TableControl::DidResize() {
  model_->DidResize(gfx::RectF(rect()));
  UpdateViewIfNeeded();
}

void TableControl::OnDraw(gfx::Graphics* gfx) {
  model_->Draw(gfx);
}

void TableControl::OnKeyPressed(const ui::KeyboardEvent& event) {
  switch (event.key_code()) {
    case KeyCode::ArrowDown:
      if (event.shift_key())
        model_->ExtendSelection(1);
      else
        model_->MoveSelection(1);
      UpdateViewIfNeeded();
      return;
    case KeyCode::ArrowUp:
      if (event.shift_key())
        model_->ExtendSelection(-1);
      else
        model_->MoveSelection(-1);
      UpdateViewIfNeeded();
      return;
  }

  observer_->OnKeyPressed(event);
}

void TableControl::OnMousePressed(const ui::MouseEvent& event) {
  model_->OnMousePressed(event);
  observer_->OnMousePressed(event);
  UpdateViewIfNeeded();
}

}  // namespace ui
