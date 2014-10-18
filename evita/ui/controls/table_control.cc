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
#include "evita/gfx/text_layout.h"
#include "evita/ui/animation/animatable_window.h"
#include "evita/ui/base/selection_model.h"
#include "evita/ui/base/table_model.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/compositor/layer_owner_delegate.h"
#include "evita/ui/controls/table_control_observer.h"
#include "evita/ui/events/event.h"
#include "evita/ui/system_metrics.h"

namespace ui {


//////////////////////////////////////////////////////////////////////
//
// PaintScheduler
//
class PaintScheduler {
  protected: PaintScheduler() = default;
  protected: virtual ~PaintScheduler() = default;

  public: virtual void SchedulePaintCanvas() = 0;

  DISALLOW_COPY_AND_ASSIGN(PaintScheduler);
};

//////////////////////////////////////////////////////////////////////
//
// CanvasPainter
//
class CanvasPainter {
  private: gfx::RectF bounds_;
  private: int canvas_bitmap_id_;
  private: bool dirty_;
  private: PaintScheduler* const paint_scheduler_;

  protected: CanvasPainter(PaintScheduler* paint_scheduler);
  public: virtual ~CanvasPainter();

  public: const gfx::RectF& bounds() const { return bounds_; }
  public: float height() const { return bounds_.height(); }
  public: float width() const { return bounds_.width(); }

  private: bool IsDirty(const gfx::Canvas* canvas) const;
  public: void MarkDirty();
  protected: virtual void OnPaintCanvas(gfx::Canvas* canvas) = 0;
  public: void Paint(gfx::Canvas* canvas);
  public: void SetBounds(const gfx::PointF& origin, const gfx::SizeF& size);
  public: void SetBounds(const gfx::RectF& new_bounds);

  DISALLOW_COPY_AND_ASSIGN(CanvasPainter);
};

CanvasPainter::CanvasPainter(PaintScheduler* paint_scheduler)
    : canvas_bitmap_id_(0), dirty_(true), paint_scheduler_(paint_scheduler) {
}

CanvasPainter::~CanvasPainter() {
}

bool CanvasPainter::IsDirty(const gfx::Canvas* canvas) const {
  return dirty_ || canvas_bitmap_id_ != canvas->bitmap_id();
}

void CanvasPainter::MarkDirty() {
  if (dirty_)
    return;
  dirty_ = true;
  paint_scheduler_->SchedulePaintCanvas();
}

void CanvasPainter::Paint(gfx::Canvas* canvas) {
  if (!IsDirty(canvas))
    return;
  canvas_bitmap_id_ = canvas->bitmap_id();
  dirty_ = false;
  gfx::Canvas::DrawingScope drawing_scope(canvas);
  OnPaintCanvas(canvas);
}

void CanvasPainter::SetBounds(const gfx::PointF& origin,
                             const gfx::SizeF& size) {
  SetBounds(gfx::RectF(origin, size));
}

void CanvasPainter::SetBounds(const gfx::RectF& new_bounds) {
  DCHECK(!new_bounds.empty());
  if (bounds_ == new_bounds)
    return;
  bounds_ = new_bounds;
  MarkDirty();
}

//////////////////////////////////////////////////////////////////////
//
// CanvasWindow
//
class CanvasWindow : public ui::AnimatableWindow,
                     protected ui::PaintScheduler,
                     protected ui::LayerOwnerDelegate {
  DECLARE_CASTABLE_CLASS(CanvasWindow, AnimatableWindow);

  private: std::unique_ptr<gfx::Canvas> canvas_;

  protected: CanvasWindow();
  protected: virtual ~CanvasWindow();

  public: gfx::Canvas* canvas() { return canvas_.get(); }

  protected: virtual void DidChangeCanvas() = 0;

  // ui::AnimationFrameHanndler
  protected: virtual void DidBeginAnimationFrame(base::Time time) override;

  // ui::PaintScheduler
  protected: virtual void SchedulePaintCanvas() override;

  // ui::LayerOwnerDelegate
  protected: virtual void DidRecreateLayer(ui::Layer* old_layer) override;

  // ui::Widget
  protected: virtual void DidHide() override;
  protected: virtual void DidRealize() override;

  DISALLOW_COPY_AND_ASSIGN(CanvasWindow);
};

CanvasWindow::CanvasWindow() {
}

CanvasWindow::~CanvasWindow() {
}

// ui::AnimationFrameHanndler
void CanvasWindow::DidBeginAnimationFrame(base::Time) {
  if (!canvas_) {
    canvas_.reset(layer()->CreateCanvas());
    DidChangeCanvas();
  } else if (GetContentsBounds() != canvas_->GetLocalBounds()) {
    canvas_->SetBounds(GetContentsBounds());
    DidChangeCanvas();
  }

  gfx::Canvas::DrawingScope drawing_scope(canvas_.get());
  OnDraw(canvas_.get());
}

// ui::PaintScheduler
void CanvasWindow::SchedulePaintCanvas() {
  SchedulePaint();
}

// ui::LayerOwnerDelegate
void CanvasWindow::DidRecreateLayer(ui::Layer*) {
  canvas_.reset();
}

// ui::Widget
void CanvasWindow::DidHide() {
  AnimatableWindow::DidHide();
  canvas_.reset();
}

void CanvasWindow::DidRealize() {
  AnimatableWindow::DidRealize();
  SetLayer(new ui::Layer());
  set_layer_owner_delegate(this);
}

namespace {

auto const kLeftMargin = 10.0f;
auto const kMarginBetweenHeaderAndRow = 3.0f;
auto const kRightMargin = 5.0f;
auto const kTopMargin = 3.0f;

std::unique_ptr<gfx::TextFormat> CreateTextFormat() {
  auto const font_size = 13.0f;
  auto text_format = std::make_unique<gfx::TextFormat>(
      L"MS Shell Dlg 2", font_size);
  common::ComPtr<IDWriteInlineObject> inline_object;
  COM_VERIFY(gfx::FactorySet::instance()->dwrite().
      CreateEllipsisTrimmingSign(*text_format.get(), &inline_object));
  DWRITE_TRIMMING trimming {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
  (*text_format.get())->SetTrimming(&trimming, inline_object);
  (*text_format.get())->SetParagraphAlignment(
      DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  return std::move(text_format);
}

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
class Item : public ui::CanvasPainter, public common::Castable {
  DECLARE_CASTABLE_CLASS(Item, common::Castable);

  private: gfx::TextFormat* const text_format_;

  protected: Item(PaintScheduler* paint_scheduler, gfx::TextFormat* text_format);
  public: virtual ~Item();

  protected: gfx::TextFormat* text_format() const { return text_format_; }

  public: Item* HitTest(const gfx::PointF& point) const;

  DISALLOW_COPY_AND_ASSIGN(Item);
};

Item::Item(PaintScheduler* paint_scheduler, gfx::TextFormat* text_format)
    : CanvasPainter(paint_scheduler), text_format_(text_format) {
}

Item::~Item() {
}

Item* Item::HitTest(const gfx::PointF& point) const {
  return bounds().Contains(point) ? const_cast<Item*>(this) : nullptr;
}

//////////////////////////////////////////////////////////////////////
//
// Column
//
class Column final : public Item {
  DECLARE_CASTABLE_CLASS(Column, Item);

  public: enum class State {
    Normal,
    Hovered,
    Pressed,
  };

  private: TableColumn data_;
  private: State state_;
  private: std::unique_ptr<gfx::TextLayout> text_layout_;

  public: Column(PaintScheduler* paint_scheduler, const TableColumn& data,
                 gfx::TextFormat* text_format);
  public: virtual ~Column();

  public: DWRITE_TEXT_ALIGNMENT alignment() const;
  public: int column_id() const { return data_.column_id; }
  public: State state() const { return state_; }
  public: const base::string16& text() const { return data_.text; }
  public: float width() const { return data_.width; }

  private: gfx::ColorF GetOverlayColor() const;
  public: void SetState(State new_state);

  // CanvasPainter
  private: virtual void OnPaintCanvas(gfx::Canvas* canvas) override;

  DISALLOW_COPY_AND_ASSIGN(Column);
};

Column::Column(PaintScheduler* paint_scheduler, const TableColumn& data,
               gfx::TextFormat* text_format)
    : Item(paint_scheduler, text_format), data_(data), state_(State::Normal) {
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

gfx::ColorF Column::GetOverlayColor() const {
  switch (state_) {
    case State::Hovered:
      return gfx::ColorF(RgbToColorF(51, 153, 255, 0.1f));
    case State::Normal:
      return gfx::ColorF(1, 1, 1);
    case State::Pressed:
      return gfx::ColorF(RgbToColorF(51, 153, 255, 0.5f));
  }
  NOTREACHED();
  return gfx::ColorF(1, 0, 0);
}

void Column::SetState(State new_state) {
  if (state_ == new_state)
    return;
  state_ = new_state;
  MarkDirty();
}

// CanvasPainter
void Column::OnPaintCanvas(gfx::Canvas* canvas) {
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds());
  canvas->AddDirtyRect(bounds());
  canvas->Clear(gfx::ColorF::White);

  // Paint column text
  {
    auto const text_bounds = bounds().Inset(5, 0);
    if (!text_layout_) {
      text_layout_ = text_format()->CreateLayout(data_.text,
                                                 text_bounds.size());
    }
    gfx::Brush text_brush(canvas, gfx::ColorF::Black);
    (*canvas)->DrawTextLayout(text_bounds.origin(), *text_layout_, text_brush,
                              D2D1_DRAW_TEXT_OPTIONS_CLIP);
  }

  // Paint separator
  {
    const auto separator = gfx::RectF(bounds().top_right() + gfx::SizeF(-1, 3),
                                      gfx::SizeF(1.0f, bounds().height() - 6));
    gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, separator);
    canvas->Clear(gfx::ColorF::LightGray);
  }

  // Pain overlay if needed
  if (state_ == State::Normal)
    return;
  canvas->FillRectangle(gfx::Brush(canvas, GetOverlayColor()), bounds());
}

//////////////////////////////////////////////////////////////////////
//
// ColumnCollection
//
class ColumnCollection : public ui::Widget, private PaintScheduler {
  DECLARE_CASTABLE_CLASS(ColumnCollection, Widget);

  private: std::vector<Column*> columns_;
  private: float column_height_;
  private: gfx::RectF canvas_bounds_;
  private: Column* hovered_column_;
  private: std::unique_ptr<gfx::TextFormat> text_format_;

  public: ColumnCollection(const std::vector<TableColumn>& columns);
  public: virtual ~ColumnCollection();

  public: float canvas_width() const;
  public: const std::vector<Column*> columns() const { return columns_; }
  public: int window_height() const;

  private: Column* HitTest(const gfx::PointF& point);
  private: void UpdateHover(Column* column);
  private: void UpdateLayoutIfNeeded();

  // ui::PaintScheduler
  protected: virtual void SchedulePaintCanvas() override;

  // ui::Widget
  private: virtual HCURSOR GetCursorAt(const gfx::Point& point) const override;
  private: virtual void OnDraw(gfx::Canvas* canvas) override;
  private: virtual void OnMouseExited(const ui::MouseEvent& event) override;
  private: virtual void OnMouseMoved(const ui::MouseEvent& event) override;

  DISALLOW_COPY_AND_ASSIGN(ColumnCollection);
};

ColumnCollection::ColumnCollection(const std::vector<TableColumn>& columns)
    : column_height_(24.0f), hovered_column_(nullptr),
      text_format_(CreateTextFormat()) {
  columns_.resize(columns.size());
  for (auto index = 0u; index < columns.size(); ++index)
    columns_[index] = new Column(this, columns[index], text_format_.get());
}

ColumnCollection::~ColumnCollection() {
  for (auto column : columns_)
    delete column;
}

float ColumnCollection::canvas_width() const {
  return canvas_bounds_.width();
}

int ColumnCollection::window_height() const {
  return static_cast<int>(column_height_ + kMarginBetweenHeaderAndRow);
}

Column* ColumnCollection::HitTest(const gfx::PointF& point) {
  UpdateLayoutIfNeeded();
  for (auto column : columns_) {
    if (column->HitTest(point))
      return column;
  }
  return nullptr;
}

void ColumnCollection::UpdateHover(Column* new_hovered_column) {
  if (hovered_column_ == new_hovered_column)
    return;
  if (hovered_column_ && hovered_column_->state() == Column::State::Hovered)
    hovered_column_->SetState(Column::State::Normal);
  hovered_column_ = new_hovered_column;
  if (hovered_column_ && hovered_column_->state() == Column::State::Normal)
    hovered_column_->SetState(Column::State::Hovered);
}

void ColumnCollection::UpdateLayoutIfNeeded() {
  if (canvas_bounds_ == GetContentsBounds())
    return;
  canvas_bounds_ = GetContentsBounds();
  gfx::PointF origin(kLeftMargin, kTopMargin);
  auto rest = columns_.size();
  gfx::PointF cell_origin(origin);
  for (auto column : columns_) {
    --rest;
    (*text_format_)->SetTextAlignment(column->alignment());
    auto const width = rest ?
        column->width() : canvas_bounds_.width() - cell_origin.x - kRightMargin;
    gfx::RectF bounds(cell_origin, gfx::SizeF(width, column_height_));
    column->SetBounds(bounds);
    DCHECK(!column->bounds().empty());
    cell_origin.x += column->width();
  }
}

// ui::PaintScheduler
void ColumnCollection::SchedulePaintCanvas() {
  SchedulePaint();
}

// ui::Widget
HCURSOR ColumnCollection::GetCursorAt(const gfx::Point& point_in) const {
  auto const point = gfx::PointF(point_in);
  auto const column = const_cast<ColumnCollection*>(this)->HitTest(point);
  if (!column)
    return nullptr;
  if (point.x < column->bounds().right - 5)
    return nullptr;
  // TODO(eval1749) We should use horizontal splitter cursor.
  return ::LoadCursor(nullptr, IDC_SIZEWE);
}

void ColumnCollection::OnDraw(gfx::Canvas* canvas) {
  auto const new_width = parent_node()->bounds().width();
  if (new_width != bounds().width())
    SetBounds(gfx::Rect(gfx::Size(new_width, window_height())));
  UpdateLayoutIfNeeded();
  for (const auto column : columns_)
    column->Paint(canvas);
}

void ColumnCollection::OnMouseExited(const ui::MouseEvent&) {
  UpdateHover(nullptr);
}

void ColumnCollection::OnMouseMoved(const ui::MouseEvent& event) {
  UpdateHover(HitTest(gfx::PointF(event.location())));
}

//////////////////////////////////////////////////////////////////////
//
// Row
//
class Row final : public Item {
  DECLARE_CASTABLE_CLASS(Row, Item);

  public: enum RowFlags {
    kNone = 0,
    kFocused = 1,
    kSelected = 1 << 1,
    kCut = 1 << 2,
    kDropHilited = 1 << 3,
    Glow = 1 << 4,
    Activating = 1 << 5,
  };

  public: enum class State {
    Normal,
    ActiveSelected,
    Hovered,
    InactiveSelected,
  };

  private: ColumnCollection* const header_;
  private: int flags_;
  private: const TableModel* const model_;
  private: int const row_id_;
  private: State state_;

  public: Row(PaintScheduler* paint_scheduler, const TableModel* model,
              ColumnCollection* columns, int row_id,
              gfx::TextFormat* text_format);
  public: virtual ~Row();

  public: int flags() const { return flags_; }
  public: int row_id() const { return row_id_; }
  public: bool selected() const { return flags_ & kSelected; }
  public: State state() const { return state_; }

  public: void SetSelected(bool selected);
  public: void SetState(State new_state);
  public: void UpdateRowFlags(int new_state, int flags_mask);

  // CanvasPainter
  private: virtual void OnPaintCanvas(gfx::Canvas* canvas) override;

  DISALLOW_COPY_AND_ASSIGN(Row);
};

Row::Row(PaintScheduler* paint_scheduler, const TableModel* model,
         ColumnCollection* columns, int row_id, gfx::TextFormat* text_format)
    : Item(paint_scheduler, text_format), header_(columns), flags_(kNone),
      model_(model), row_id_(row_id), state_(State::Normal) {
}

Row::~Row() {
}

void Row::SetSelected(bool selected) {
  UpdateRowFlags(selected ? kSelected : 0, kSelected);
}

void Row::SetState(State new_state) {
  if (state_ == new_state)
    return;
  MarkDirty();
  state_ = new_state;
}

void Row::UpdateRowFlags(int new_flags_bits, int new_flags_mask) {
  auto const new_flag = (flags_ & ~new_flags_mask) | new_flags_bits;
  if (flags_ == new_flag)
    return;
  MarkDirty();
  flags_ = new_flag;
}

// CanvasPainter
void Row::OnPaintCanvas(gfx::Canvas* canvas) {
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds());
  canvas->AddDirtyRect(bounds());
  auto const kPadding = 2.0f;
  auto const bgcolor = gfx::ColorF(gfx::ColorF::White);
  auto const color = gfx::ColorF(gfx::ColorF::Black);
  canvas->FillRectangle(gfx::Brush(canvas, bgcolor), bounds());
  gfx::Brush text_brush(canvas, color);
  gfx::PointF cell_origin(bounds().origin());
  auto rest = header_->columns().size();
  for (auto column : header_->columns()) {
    --rest;
    auto const text = model_->GetCellText(row_id(), column->column_id());
    (*text_format())->SetTextAlignment(column->alignment());
    auto const width = rest ?
        column->width() : bounds().width() - cell_origin.x - kRightMargin;
    auto const cell_bounds = gfx::RectF(cell_origin,
                                        gfx::SizeF(width, height()));
    auto const text_bounds = cell_bounds.Inset(kPadding, kPadding);
    (*canvas)->DrawText(text.data(), static_cast<uint32_t>(text.length()),
                        *text_format(), text_bounds, text_brush);
    cell_origin.x += column->width();
  }

  switch (state_) {
    case State::ActiveSelected:
      canvas->FillRectangle(
          gfx::Brush(canvas, RgbToColorF(51, 153, 255, 0.2f)), bounds());
      canvas->DrawRectangle(
          gfx::Brush(canvas, RgbToColorF(51, 153, 255, 1.0f)), bounds());
      break;
    case State::Hovered:
      canvas->FillRectangle(
          gfx::Brush(canvas, RgbToColorF(51, 153, 255, 0.1f)), bounds());
      canvas->DrawRectangle(
          gfx::Brush(canvas, RgbToColorF(51, 153, 255, 1.0f)), bounds());
       break;
    case State::InactiveSelected:
      canvas->FillRectangle(
          gfx::Brush(canvas, RgbToColorF(191, 205, 191, 0.2f)), bounds());
      canvas->DrawRectangle(
          gfx::Brush(canvas, RgbToColorF(191, 205, 191, 1.0f)), bounds());
       break;
    case State::Normal:
      break;
    default:
      NOTREACHED();
      break;
  }
}

//////////////////////////////////////////////////////////////////////
//
// RowCompare
//
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

//////////////////////////////////////////////////////////////////////
//
// RowCollection
//
class RowCollection final : public CanvasWindow, public TableModelObserver {
  DECLARE_CASTABLE_CLASS(RowCollection, CanvasWindow);

  private: ColumnCollection* header_;
  private: Row* hovered_row_;
  private: const TableModel* model_;
  private: bool need_sort_rows_;
  private: bool need_update_layout_;
  private: bool need_update_selection_;
  private: TableControlObserver* const observer_;
  private: std::vector<Row*> rows_;
  private: std::unordered_map<int, Row*> row_map_;
  private: float const row_height_;
  private: SelectionModel selection_;
  private: std::unique_ptr<gfx::TextFormat> text_format_;

  public: RowCollection(const TableModel* model,
                        ColumnCollection* columns,
                        TableControlObserver* observer);
  public: virtual ~RowCollection();

  private: Row* CreateRow(int row_id);
  private: void ExtendSelection(int direction);
  private: const Row* GetRowById(int row_id) const;
  private: int GetRowIndex(const Row* row);
  public: int GetRowFlags(int row_id) const;
  private: Row* HitTest(const gfx::PointF& point);
  private: void MoveSelection(int direction);
  private: void NeedSortRows();
  private: void NeedUpdateLayout();
  private: void NeedUpdateSelection();
  public: void Select(int row_id);
  public: void SortRowsIfNeeded(size_t column_id);
  private: void UpdateHover(Row* new_hovered_row);
  private: void UpdateLayoutIfNeeded();
  private: void UpdateRowFlags();
  private: void UpdateSelectionIfNeeded();

  // ui::AnimationFrameHanndler
  private: virtual void DidBeginAnimationFrame(base::Time time) override;

  // ui::CanavsWindow
  private: virtual void DidChangeCanvas() override;

  // ui::TableModelObserver
  public: virtual void DidAddRow(int row_id) override;
  public: virtual void DidChangeRow(int row_id) override;
  public: virtual void DidRemoveRow(int row_id) override;

  // ui::Widget
  private: virtual void DidKillFocus(ui::Widget* focused_window) override;
  private: virtual void DidRealize() override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual void OnDraw(gfx::Canvas* canvas) override;
  private: virtual void OnKeyPressed(const KeyEvent& event) override;
  private: virtual void OnMouseExited(const ui::MouseEvent& event) override;
  private: virtual void OnMouseMoved(const ui::MouseEvent& event) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;

  DISALLOW_COPY_AND_ASSIGN(RowCollection);
};

RowCollection::RowCollection(const TableModel* model,
                             ColumnCollection* columns,
                             TableControlObserver* observer)
    : header_(columns), hovered_row_(nullptr),
      model_(model), need_sort_rows_(true), need_update_layout_(true),
      need_update_selection_(false), observer_(observer), row_height_(24.0f),
      selection_(model->GetRowCount()),
      text_format_(CreateTextFormat()) {
  auto const num_rows = static_cast<size_t>(model->GetRowCount());
  for (auto index = 0u; index < num_rows; ++index) {
    auto const row_id = model_->GetRowId(static_cast<int>(index));
    auto const row = CreateRow(row_id);
    rows_.push_back(row);
    row_map_[row_id] = row;
  }
}

RowCollection::~RowCollection() {
  for (auto const row : rows_)
    delete row;
}

Row* RowCollection::CreateRow(int row_id) {
  return new Row(this, model_, header_, row_id, text_format_.get());
}

void RowCollection::ExtendSelection(int direction) {
  selection_.Extend(direction);
  NeedUpdateSelection();
}

const Row* RowCollection::GetRowById(int row_id) const {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return nullptr;
  }
  return present->second;
}

int RowCollection::GetRowIndex(const Row* present) {
  SortRowsIfNeeded(0u);
  auto index = 0;
  for (auto row : rows_) {
    if (row == present)
      return index;
    ++index;
  }
  return -1;
}

int RowCollection::GetRowFlags(int row_id) const {
  auto row = GetRowById(row_id);
  return row ? row->flags() : 0;
}

Row* RowCollection::HitTest(const gfx::PointF& point) {
  UpdateLayoutIfNeeded();
  for (auto row : rows_) {
    if (row->HitTest(point))
      return row;
  }
  return nullptr;
}

void RowCollection::MoveSelection(int direction) {
  selection_.Move(direction);
  NeedUpdateSelection();
}

void RowCollection::NeedSortRows() {
  need_sort_rows_ = true;
  NeedUpdateLayout();
}

void RowCollection::NeedUpdateLayout() {
  RequestAnimationFrame();
  need_update_layout_ = true;
}

void RowCollection::NeedUpdateSelection() {
  RequestAnimationFrame();
  need_update_selection_ = true;
}

void RowCollection::Select(int row_id) {
  auto row = GetRowById(row_id);
  if (!row)
    return;
  auto index = GetRowIndex(row);
  if (index < 0)
    return;
  selection_.CollapseTo(index);
  NeedUpdateSelection();
}

void RowCollection::SortRowsIfNeeded(size_t column_index) {
  if (!need_sort_rows_)
    return;
  need_sort_rows_ = false;
  UpdateRowFlags();
  std::sort(rows_.begin(), rows_.end(),
            RowCompare(model_, header_->columns()[column_index]->column_id()));
  selection_.Clear();
  auto index = 0;
  for (auto const row : rows_) {
    if (row->selected())
      selection_.Add(index);
    ++index;
  }
  NeedUpdateLayout();
}

void RowCollection::UpdateHover(Row* new_hovered_row) {
  if (hovered_row_ == new_hovered_row)
    return;
  if (hovered_row_ && hovered_row_->state() == Row::State::Hovered)
    hovered_row_->SetState(Row::State::Normal);
  hovered_row_ = new_hovered_row;
  if (hovered_row_ && hovered_row_->state() == Row::State::Normal)
    hovered_row_->SetState(Row::State::Hovered);
}

void RowCollection::UpdateLayoutIfNeeded() {
  SortRowsIfNeeded(0u);
  if (!need_update_layout_)
    return;
  need_update_layout_ = false;
  auto bounds = gfx::RectF(gfx::PointF(kLeftMargin, 0.0f),
                           gfx::PointF(GetContentsBounds().right - kRightMargin,
                                       row_height_));
  for (auto row : rows_) {
    row->SetBounds(bounds);
    bounds = bounds.Offset(0.0f, row_height_);
  }
}

void RowCollection::UpdateRowFlags() {
  auto index = 0;
  for(auto row : rows_) {
    row->SetSelected(selection_.IsSelected(index));
    ++index;
  }
}

void RowCollection::UpdateSelectionIfNeeded() {
  if (!need_update_selection_)
    return;
  need_update_selection_ = false;
  UpdateRowFlags();
  for (auto row : rows_) {
    if (row->selected()) {
      row->SetState(has_focus() ? Row::State::ActiveSelected :
                                  Row::State::InactiveSelected);
    } else if (row == hovered_row_) {
      row->SetState(Row::State::Hovered);
    } else {
      row->SetState(Row::State::Normal);
    }
  }
}

// ui::AnimationFrameHanndler
void RowCollection::DidBeginAnimationFrame(base::Time time) {
  auto const new_bottom_right = parent_node()->bounds().bottom_right();
  if (new_bottom_right != bounds().bottom_right()) {
    SetBounds(gfx::Point(0, static_cast<int>(header_->window_height())),
              new_bottom_right);
  }
  CanvasWindow::DidBeginAnimationFrame(time);
}

// ui::CanvasWindow
void RowCollection::DidChangeCanvas() {
  NeedUpdateLayout();
}

// ui::TableModelObserver
void RowCollection::DidAddRow(int row_id) {
  auto const row = CreateRow(row_id);
  rows_.push_back(row);
  row_map_[row_id] = row;
  // Notify selection about newly added row.
  selection_.DidAddItem(static_cast<int>(rows_.size() - 1));
  NeedSortRows();
  // Selection may not be changed by this addition.
  NeedUpdateSelection();
}

void RowCollection::DidChangeRow(int row_id) {
  auto row = GetRowById(row_id);
  if (!row)
    return;
  const_cast<Row*>(row)->MarkDirty();
  NeedSortRows();
}

void RowCollection::DidRemoveRow(int row_id) {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return;
  }
  auto const row = present->second;
  row_map_.erase(present);

  if (hovered_row_ == row)
    hovered_row_ = nullptr;

  auto index = 0;
  auto row_pos = rows_.end();
  gfx::RectF new_rect;
  for (auto it = rows_.begin(); it < rows_.end(); ++it){
    auto row = *it;
    if (row->row_id() == row_id) {
      row_pos = it;
      selection_.DidRemoveItem(index);
      NeedUpdateLayout();
      break;
    }
    ++index;
  }
  DCHECK(row_pos != rows_.end());
  rows_.erase(row_pos);
  delete row;
  NeedUpdateLayout();
}

// ui::Widget
void RowCollection::DidKillFocus(Widget*) {
  // Change selected rows to inactive selected color.
  RequestAnimationFrame();
  NeedUpdateSelection();
}

void RowCollection::DidRealize() {
  struct Local {
    static Layer* GetParentLayer(Widget* widget) {
      for (auto runner = widget->parent_node(); runner;
           runner = runner->parent_node()) {
        if (auto layer = runner->layer())
          return layer;
      }
      NOTREACHED();
      return nullptr;
    }
  };

  CanvasWindow::DidRealize();
  Local::GetParentLayer(this)->AppendLayer(layer());
}

void RowCollection::DidSetFocus(Widget*) {
  // Change selected rows to active selected color.
  RequestAnimationFrame();
  NeedUpdateSelection();
}

void RowCollection::OnDraw(gfx::Canvas* canvas) {
  if (!visible())
    return;
  if (canvas != this->canvas()) {
    // Called from parent window's |OnDraw()|.
    RequestAnimationFrame();
    return;
  }
  if (header_->canvas_width() != canvas->width()) {
    // |TableControl| is changed but |ColumnCollection| isn't update yet,
    // We'll update |RowCollection| in next animation frame.
    RequestAnimationFrame();
    NeedUpdateLayout();
    return;
  }
  if (selection_.empty() && rows_.size()) {
    // If nothing is selected, we select the first row.
    selection_.CollapseTo(0);
    NeedUpdateSelection();
  }

  UpdateLayoutIfNeeded();
  UpdateSelectionIfNeeded();

  if (canvas->should_clear())
    canvas->Clear(gfx::ColorF(gfx::ColorF::White));

  for (const auto row : rows_)
    row->Paint(canvas);
}

void RowCollection::OnKeyPressed(const ui::KeyEvent& event) {
  switch (event.key_code()) {
    case KeyCode::ArrowDown:
      if (event.shift_key())
        ExtendSelection(1);
      else
        MoveSelection(1);
      return;
    case KeyCode::ArrowUp:
      if (event.shift_key())
        ExtendSelection(-1);
      else
        MoveSelection(-1);
      return;
  }

  observer_->OnKeyPressed(event);
}

void RowCollection::OnMouseExited(const ui::MouseEvent&) {
  UpdateHover(nullptr);
}

void RowCollection::OnMouseMoved(const ui::MouseEvent& event) {
  UpdateHover(HitTest(gfx::PointF(event.location())));
}

void RowCollection::OnMousePressed(const ui::MouseEvent& event) {
  observer_->OnMousePressed(event);
  if (!event.is_left_button() || event.click_count())
    return;
  if (!has_focus())
    RequestFocus();
  auto const row = HitTest(gfx::PointF(event.location()));
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
  NeedUpdateSelection();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TableControl::View
//
class TableControl::View final : public ui::Widget {
  DECLARE_CASTABLE_CLASS(View, Widget);

  private: ColumnCollection* header_;
  private: RowCollection* row_collection_;

  public: View(ui::Widget* widget_, const std::vector<TableColumn>& columns,
               const TableModel* model, TableControlObserver* observer);
  public: virtual ~View();

  public: RowCollection* row_collection() { return row_collection_; }

  DISALLOW_COPY_AND_ASSIGN(View);
};

TableControl::View::View(ui::Widget* widget,
                         const std::vector<TableColumn>& columns,
                         const TableModel* model,
                         TableControlObserver* observer)
    : header_(new ColumnCollection(columns)),
      row_collection_(new RowCollection(model, header_, observer)) {
  widget->AppendChild(header_);
  header_->SetBounds(gfx::Rect(gfx::Size(1, 1)));
  widget->AppendChild(row_collection_);
  row_collection_->SetBounds(gfx::Rect(gfx::Size(1, 1)));
}

TableControl::View::~View() {
}

//////////////////////////////////////////////////////////////////////
//
// TableControl
//
TableControl::TableControl(const std::vector<TableColumn>& columns,
                           const TableModel* model,
                           TableControlObserver* observer)
    : view_(new View(this, columns, model, observer)) {
}

TableControl::~TableControl() {
}

TableModelObserver* TableControl::GetTableModelObserver() {
  return static_cast<TableModelObserver*>(view_->row_collection());
}

int TableControl::GetRowState(int row_id) const {
  return view_->row_collection()->GetRowFlags(row_id);
}

void TableControl::Select(int row_id) {
  view_->row_collection()->Select(row_id);
}

// ui::Widget
void TableControl::DidSetFocus(Widget*) {
  // Transfer focus to |RowCollection|. |TableControl| is a just container of
  // header, row collection and scroll bar.
  view_->row_collection()->RequestFocus();
}

}  // namespace ui
