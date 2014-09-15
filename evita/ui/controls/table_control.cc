// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/table_control.h"

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "common/castable.h"
#include "common/win/scoped_comptr.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/text_format.h"
#include "evita/ui/base/selection_model.h"
#include "evita/ui/base/table_model.h"
#include "evita/ui/controls/table_control_observer.h"
#include "evita/ui/events/event.h"

namespace ui {

namespace {

gfx::ColorF RgbToColorF(int red, int green, int blue, float alpha) {
  return gfx::ColorF(static_cast<float>(red) / 255,
                     static_cast<float>(green) / 255,
                     static_cast<float>(blue) / 255,
                     alpha);
}

//////////////////////////////////////////////////////////////////////
//
// Item
//
class Item : public common::Castable {
  DECLARE_CASTABLE_CLASS(Item, common::Castable);

  private: gfx::RectF bounds_;

  public: Item();
  public: virtual ~Item();

  public: const gfx::RectF& bounds() const { return bounds_; }
  public: void set_rect(const gfx::RectF& rect) { bounds_ = rect; }

  public: Item* HitTest(const gfx::PointF& point) const;

  DISALLOW_COPY_AND_ASSIGN(Item);
};

Item::Item() {
}

Item::~Item() {
}

Item* Item::HitTest(const gfx::PointF& point) const {
  return bounds_.Contains(point) ? const_cast<Item*>(this) : nullptr;
}

//////////////////////////////////////////////////////////////////////
//
// Column
//
class Column final : public Item {
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
class Row final : public Item {
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

struct RowCompare {
  const TableModel* model_;
  int column_id_;

  RowCompare(const TableModel* model, int column_id)
      : column_id_(column_id), model_(model) {
  }

  bool operator() (const Row* a, const Row* b) const {
    const auto& a_text = model_->GetCellText(a->row_id(), column_id_);
    const auto& b_text = model_->GetCellText(b->row_id(), column_id_);
    return a_text < b_text;
  }
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TableControl::Impl
//
class TableControl::Impl final {
  private: gfx::RectF bounds_;
  private: std::vector<Column*> columns_;
  private: gfx::RectF dirty_rects_;
  private: bool has_focus_;
  private: Row* hover_row_;
  private: const TableModel* model_;
  private: std::vector<Row*> rows_;
  private: std::unordered_map<int, Row*> row_map_;
  private: float row_height_;
  private: SelectionModel selection_;
  private: std::unique_ptr<gfx::TextFormat> text_format_;
  // Impl uses |widget_| for managing mouse capture.
  private: Widget* widget_;

  public: Impl(Widget* widget, const std::vector<TableColumn>& columns,
               const TableModel* model);
  public: ~Impl();

  private: void AddDirtyRect(const gfx::RectF& dirty_rect);
  public: void DidAddRow(int row_id);
  public: void DidChangeRow(int row_id);
  public: void DidHide();
  public: void DidKillFocus(ui::Widget* focused_window);
  public: void DidRemoveRow(int row_id);
  public: void DidChangeBounds(const gfx::RectF& rect);
  public: void DidShow();
  public: void DidSetFocus(ui::Widget* last_focused);
  public: void Draw(gfx::Canvas* canvas) const;
  private: void DrawHeaderRow(gfx::Canvas* canvas) const;
  private: void DrawRow(gfx::Canvas* canvas, const Row* row) const;
  public: void ExtendSelection(int direction);
  private: const Row* GetRowById(int row_id) const;
  private: int GetRowIndex(const Row* row) const;
  public: int GetRowState(int row_id) const;
  private: Item* HitTest(const gfx::PointF& point) const;
  public: void MakeSelectionViewDirty();
  public: void MoveSelection(int direction);
  public: void OnMouseExited(const ui::MouseEvent& event);
  public: void OnMouseMoved(const ui::MouseEvent& event);
  public: void OnMousePressed(const ui::MouseEvent& event);
  public: gfx::RectF ResetDirtyRect();
  public: void Select(int row_id);

  // Sort rows
  private: void SortRows();

  // Update layout of rows after number of rows changed or order of row is
  // changed.
  private: void UpdateLayout();

  private: void UpdateSelectionView();

  DISALLOW_COPY_AND_ASSIGN(Impl);
};

TableControl::Impl::Impl(Widget* widget,
                         const std::vector<TableColumn>& columns,
                         const TableModel* model)
    : has_focus_(false),
      hover_row_(nullptr),
      model_(model),
      row_height_(24.0f),
      selection_(model->GetRowCount()),
      text_format_(new gfx::TextFormat(L"MS Shell Dlg 2", 14)),
      widget_(widget) {
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
  for (auto index = 0u; index < columns.size(); ++index)
    columns_[index] = new Column(columns[index]);
  SortRows();
}

TableControl::Impl::~Impl() {
  for (auto row : rows_)
    delete row;
  for (auto column : columns_)
    delete column;
}

void TableControl::Impl::AddDirtyRect(
    const gfx::RectF& dirty_rect) {
  DCHECK(!dirty_rect.empty());
  dirty_rects_ += dirty_rect;
}

void TableControl::Impl::DidAddRow(int row_id) {
  auto const row = new Row(row_id);
  rows_.push_back(row);
  row_map_[row_id] = row;
  SortRows();

  // Notify selection about newly added row.
  auto const it = std::find(rows_.begin(), rows_.end(), row);
  DCHECK(rows_.end() != it);
  auto const index = static_cast<int>(it - rows_.begin());
  selection_.DidAddItem(index);
}

void TableControl::Impl::DidChangeBounds(
    const gfx::RectF& new_bounds) {
  bounds_ = new_bounds;
  UpdateLayout();
}

void TableControl::Impl::DidChangeRow(int row_id) {
  auto row = GetRowById(row_id);
  if (!row)
    return;
  AddDirtyRect(row->bounds());
}

void TableControl::Impl::DidHide() {
  if (!hover_row_)
    return;
  widget_->ReleaseCapture();
  hover_row_ = nullptr;
}

void TableControl::Impl::DidKillFocus(ui::Widget*) {
  has_focus_ = false;
  MakeSelectionViewDirty();
}

void TableControl::Impl::DidRemoveRow(int row_id) {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return;
  }
  auto const row = present->second;
  row_map_.erase(present);

  auto index = 0;
  auto row_pos = rows_.end();
  gfx::RectF new_rect;
  for (auto it = rows_.begin(); it < rows_.end(); ++it){
    auto row = *it;
    if (row->row_id() == row_id) {
      row_pos = it;
      selection_.DidRemoveItem(index);
      new_rect = row->bounds();
      AddDirtyRect(new_rect);
    } else if (!new_rect.empty()) {
      auto const next_new_rect = row->bounds();
      AddDirtyRect(next_new_rect);
      row->set_rect(new_rect);
      new_rect = next_new_rect;
    }
    ++index;
  }
  DCHECK(row_pos != rows_.end());
  rows_.erase(row_pos);
  delete row;
  UpdateSelectionView();
}

void TableControl::Impl::DidSetFocus(ui::Widget*) {
  has_focus_ = true;
  if (selection_.empty() && rows_.size()) {
    selection_.CollapseTo(0);
    UpdateSelectionView();
    return;
  }
  MakeSelectionViewDirty();
}

void TableControl::Impl::DidShow() {
  dirty_rects_ = bounds_;
}

void TableControl::Impl::Draw(gfx::Canvas* canvas) const {
  gfx::Brush fill_brush(canvas, gfx::ColorF::White);

  // Fill top edge
  canvas->FillRectangle(fill_brush, gfx::RectF(
      gfx::PointF(bounds_.left, bounds_.top),
      gfx::SizeF(bounds_.width(), columns_[0]->bounds().top - bounds_.top)));

  DrawHeaderRow(canvas);

  // Draw rows
  auto last_row = static_cast<Row*>(nullptr);
  for (auto row : rows_) {
    DrawRow(canvas, row);
    canvas->Flush();
    last_row = row;
    if (row->bounds().bottom >= bounds_.bottom)
      break;
  }

  // Fill edge
  if (!last_row) {
    canvas->FillRectangle(fill_brush, gfx::RectF(
        gfx::PointF(bounds_.left, columns_[0]->bounds().bottom),
        gfx::PointF(bounds_.right, bounds_.bottom)));
    return;
  }

  // Fill between header and rows
  canvas->FillRectangle(fill_brush, gfx::RectF(
      gfx::PointF(bounds_.left, columns_.front()->bounds().bottom),
      gfx::PointF(bounds_.right, rows_.front()->bounds().top)));

  // Fill left edge
  canvas->FillRectangle(fill_brush, gfx::RectF(
    bounds_.left, bounds_.top,
    last_row->bounds().left, last_row->bounds().bottom));

  // Fill right edge
  auto const right_edge = gfx::RectF(
    last_row->bounds().right, rows_.front()->bounds().top,
    bounds_.right, last_row->bounds().bottom);
  if (!right_edge.empty())
    canvas->FillRectangle(fill_brush, right_edge);

  // Fill bottom edge
  if (last_row->bounds().bottom >= bounds_.bottom)
    return;
  canvas->FillRectangle(fill_brush, gfx::RectF(
      bounds_.left, last_row->bounds().bottom,
      bounds_.right, bounds_.bottom));
}

void TableControl::Impl::DrawHeaderRow(gfx::Canvas* canvas) const {
  canvas->FillRectangle(gfx::Brush(canvas, gfx::ColorF::White),
                        gfx::RectF(columns_.front()->bounds().origin(),
                                   columns_.back()->bounds().bottom_right()));
  gfx::Brush text_brush(canvas, gfx::ColorF::Black);
  gfx::Brush gray_brush(canvas, gfx::ColorF::LightGray);
  auto column_index = 0u;
  for (auto column : columns_) {
    ++column_index;
    auto const rect = column->bounds();
    auto const text = column->text();
    canvas->DrawText(*text_format_, text_brush, rect, text);
    canvas->DrawLine(gray_brush,
                     gfx::PointF(rect.right - 5, rect.top),
                     gfx::PointF(rect.right - 5, rect.bottom), 0.5f);
  }
}

void TableControl::Impl::DrawRow(gfx::Canvas* canvas, const Row* row) const {
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, row->bounds());
  auto const kPadding = 2.0f;
  auto const bgcolor = gfx::ColorF(gfx::ColorF::White);
  auto const color = gfx::ColorF(gfx::ColorF::Black);
  canvas->FillRectangle(gfx::Brush(canvas, bgcolor), row->bounds());
  gfx::Brush textBrush(canvas, color);
  gfx::PointF cell_origin(row->bounds().origin());
  auto column_index = 0u;
  for (auto column : columns_) {
    auto const text = model_->GetCellText(row->row_id(), column->column_id());
    (*text_format_)->SetTextAlignment(column->alignment());
    auto const width = column_index == columns_.size() ?
        bounds_.width() - cell_origin.x : column->width();
    gfx::RectF rect(cell_origin, gfx::SizeF(width, row_height_));
    rect.left += kPadding;
    rect.top += kPadding;
    rect.right -= kPadding;
    rect.bottom -= kPadding;
    (*canvas)->DrawText(text.data(), static_cast<uint32_t>(text.length()),
                     *text_format_, rect, textBrush);
    cell_origin.x += column->width();
  }
  if (row->selected()) {
    canvas->FillRectangle(gfx::Brush(canvas, has_focus_ ?
                                     RgbToColorF(51, 153, 255, 0.2f) :
                                     RgbToColorF(191, 205, 191, 0.2f)),
                       row->bounds());
    canvas->DrawRectangle(gfx::Brush(canvas, has_focus_ ?
                                     RgbToColorF(51, 153, 255, 1.0f) :
                                     RgbToColorF(191, 205, 191, 1.0f)),
                       row->bounds());
    return;
  }

  if (row != hover_row_)
    return;
  canvas->FillRectangle(gfx::Brush(canvas, RgbToColorF(51, 153, 255, 0.1f)),
                        row->bounds());
  canvas->DrawRectangle(gfx::Brush(canvas, RgbToColorF(51, 153, 255, 1.0f)),
                        row->bounds());
}

void TableControl::Impl::ExtendSelection(int direction) {
  selection_.Extend(direction);
  UpdateSelectionView();
}

const Row* TableControl::Impl::GetRowById(int row_id) const {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return nullptr;
  }
  return present->second;
}

int TableControl::Impl::GetRowIndex(const Row* present) const {
  auto index = 0;
  for (auto row : rows_) {
    if (row == present)
      return index;
    ++index;
  }
  return -1;
}

int TableControl::Impl::GetRowState(int row_id) const {
  auto row = GetRowById(row_id);
  return row ? row->state() : 0;
}

Item* TableControl::Impl::HitTest(
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

void TableControl::Impl::MakeSelectionViewDirty() {
  auto index = 0;
  for(auto row : rows_) {
    if (selection_.IsSelected(index)) {
      row->UpdateState(row->selected() ? Row::kSelected : 0, Row::kSelected);
      AddDirtyRect(row->bounds());
    }
    ++index;
  }
}

void TableControl::Impl::MoveSelection(int direction) {
  selection_.Move(direction);
  UpdateSelectionView();
}

void TableControl::Impl::OnMouseExited(const ui::MouseEvent&) {
  if (!hover_row_)
    return;
  AddDirtyRect(hover_row_->bounds());
  hover_row_ = nullptr;
  widget_->ReleaseCapture();
}

void TableControl::Impl::OnMouseMoved(
    const ui::MouseEvent& event) {
  auto const item = HitTest(gfx::PointF(event.location()));
  if (!item)
   return;

  auto const new_hover_row = item->as<Row>();
  if (hover_row_) {
    if (hover_row_ == new_hover_row)
      return;
    AddDirtyRect(hover_row_->bounds());
    if (new_hover_row)
      AddDirtyRect(new_hover_row->bounds());
    else
      widget_->ReleaseCapture();
    hover_row_ = new_hover_row;
    return;
  }

  if (!new_hover_row)
    return;
  hover_row_ = new_hover_row;
  widget_->SetCapture();
}

void TableControl::Impl::OnMousePressed(
    const ui::MouseEvent& event) {
  auto const item = HitTest(gfx::PointF(event.location()));
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

gfx::RectF TableControl::Impl::ResetDirtyRect() {
  auto dirty_rect = dirty_rects_;
  dirty_rects_ = gfx::RectF();
  return dirty_rect;
}

void TableControl::Impl::Select(int row_id) {
  auto row = GetRowById(row_id);
  if (!row)
    return;
  auto index = GetRowIndex(row);
  if (index < 0)
    return;
  selection_.CollapseTo(index);
  UpdateSelectionView();
}

void TableControl::Impl::SortRows() {
  std::sort(rows_.begin(), rows_.end(),
            RowCompare(model_, columns_[0]->column_id()));
  UpdateLayout();
}

void TableControl::Impl::UpdateLayout() {
  if (bounds_.empty()) {
    // Control isn't realized yet.
    return;
  }

  auto const kLeftMargin = 10.0f;
  auto const kMarginBetweenHeaderAndRow = 3.0f;
  auto const kTopMargin = 3.0f;

  gfx::PointF origin(bounds_.left + kLeftMargin, bounds_.top + kTopMargin);

  // Layout columns
  auto column_index = 0u;
  gfx::PointF cell_origin(origin);
  for (auto column : columns_) {
    ++column_index;
    (*text_format_)->SetTextAlignment(column->alignment());
    auto const width = column_index == columns_.size() ?
        bounds_.width() - cell_origin.x : column->width();
    gfx::RectF rect(cell_origin, gfx::SizeF(width, row_height_));
    column->set_rect(rect);
    DCHECK(!column->bounds().empty());
    cell_origin.x += column->width();
  }

  // Layout rows
  origin.y += row_height_ + kMarginBetweenHeaderAndRow;
  for (auto row : rows_) {
    row->set_rect(gfx::RectF(origin.x, origin.y,
                             bounds_.right, origin.y + row_height_));
    DCHECK(!row->bounds().empty());
    origin.y += row->bounds().height();
  }

  dirty_rects_ = bounds_;
}

void TableControl::Impl::UpdateSelectionView() {
  auto index = 0;
  for(auto row : rows_) {
    if (row->selected() != selection_.IsSelected(index)) {
      row->UpdateState(row->selected() ? 0 : Row::kSelected, Row::kSelected);
      AddDirtyRect(row->bounds());
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
    : impl_(new Impl(this, columns, model)),
      observer_(observer) {
}

TableControl::~TableControl() {
}

int TableControl::GetRowState(int row_id) const {
  return impl_->GetRowState(row_id);
}

void TableControl::RenderIfNeeded(gfx::Canvas* canvas) {
  if (!visible() || !is_realized())
    return;
  auto dirty_rect = impl_->ResetDirtyRect();
  if (dirty_rect.empty())
    return;
  gfx::Canvas::DrawingScope drawing_scope(canvas);
  canvas->AddDirtyRect(dirty_rect);
  impl_->Draw(canvas);
}

void TableControl::Select(int row_id) {
  impl_->Select(row_id);
}

// ui::TableModelObserver
void TableControl::DidAddRow(int row_id) {
  impl_->DidAddRow(row_id);
}

void TableControl::DidChangeRow(int row_id) {
  impl_->DidChangeRow(row_id);
}

void TableControl::DidRemoveRow(int row_id) {
  impl_->DidRemoveRow(row_id);
}

// ui::Widget
void TableControl::DidHide() {
  impl_->DidHide();
}

void TableControl::DidKillFocus(ui::Widget* focus_widget) {
  impl_->DidKillFocus(focus_widget);
}

void TableControl::DidRealize() {
  impl_->DidChangeBounds(GetContentsBounds());
}

void TableControl::DidChangeBounds() {
  impl_->DidChangeBounds(GetContentsBounds());
}

void TableControl::DidSetFocus(ui::Widget* widget) {
  impl_->DidSetFocus(widget);
}

void TableControl::DidShow() {
  impl_->DidShow();
}

void TableControl::OnKeyPressed(const ui::KeyboardEvent& event) {
  switch (event.key_code()) {
    case KeyCode::ArrowDown:
      if (event.shift_key())
        impl_->ExtendSelection(1);
      else
        impl_->MoveSelection(1);
      return;
    case KeyCode::ArrowUp:
      if (event.shift_key())
        impl_->ExtendSelection(-1);
      else
        impl_->MoveSelection(-1);
      return;
  }

  observer_->OnKeyPressed(event);
}

void TableControl::OnMouseExited(const ui::MouseEvent& event) {
  impl_->OnMouseExited(event);
}

void TableControl::OnMouseMoved(const ui::MouseEvent& event) {
  impl_->OnMouseMoved(event);
}

void TableControl::OnMousePressed(const ui::MouseEvent& event) {
  if (!has_focus())
    RequestFocus();
  impl_->OnMousePressed(event);
  observer_->OnMousePressed(event);
}

}  // namespace ui
