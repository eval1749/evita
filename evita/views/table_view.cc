// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/table_view.h"

#include <commctrl.h>
#include <algorithm>

#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "evita/dom/text/document.h"
#include "evita/editor/dom_lock.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/text/buffer.h"
#include "evita/ui/animation/animator.h"
#include "evita/ui/base/table_model_observer.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/controls/table_control.h"
#include "evita/views/icon_cache.h"
#include "evita/views/table_view_model.h"
#include "evita/vi_Frame.h"

extern HINSTANCE g_hInstance;

namespace views {

namespace {

float CellWidth(const TableViewModel::Cell& cell) {
  // TODO(yosi) We should get character width from ListView control.
  const auto kCharWidth = 6.0f;
  const auto kLeftMargin = 5.0f;
  const auto kRightMargin = 5.0f;
  return cell.text().length() * kCharWidth + kLeftMargin + kRightMargin;
}

std::vector<ui::TableColumn> BuildColumns(const TableViewModel::Row* row) {
  // TODO(yosi) We should specify minium/maximum column width from model.
  auto min_width = 200.0f;
  auto max_width = 300.0f;
  std::vector<ui::TableColumn> columns;
  for (auto& cell : row->cells()) {
    ui::TableColumn column;
    column.alignment = ui::TableColumn::Alignment::Left;
    column.column_id = static_cast<int>(columns.size());
    column.text = cell.text();
    column.width = std::max(std::min(CellWidth(cell), max_width), min_width);
    columns.push_back(column);
    min_width = 0.0f;
  }
  return std::move(columns);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TableView
//
TableView::TableView(WindowId window_id, dom::Document* document)
    : ContentWindow(window_id),
      control_(nullptr),
      document_(document),
      model_(new TableViewModel()),
      should_update_model_(true) {
}

TableView::~TableView() {
  document_->buffer()->RemoveObserver(this);
}

std::vector<int> TableView::GetRowStates(
    const std::vector<base::string16>& keys) const {
  std::vector<int> states;
  for (auto key : keys) {
    auto const row = model_->FindRow(key);
    auto const state = row ? control_->GetRowState(row->row_id()) : 0;
    states.push_back(state);
  }
  return std::move(states);
}

void TableView::Redraw() {
  UI_ASSERT_DOM_LOCKED();
  auto new_model = UpdateModelIfNeeded();
  if (!new_model)
    return;
  UpdateControl(std::move(new_model));
}

void TableView::UpdateControl(std::unique_ptr<TableViewModel> new_model) {

  if (*model_->header_row() == *new_model->header_row()) {
    auto old_model = std::move(model_);
    model_ = std::move(new_model);
    auto const observer = static_cast<ui::TableModelObserver*>(control_);
    auto max_row_id = 0;
    std::vector<TableViewModel::Row*> removed_rows;
    for (auto old_row : old_model->rows()) {
      max_row_id = std::max(max_row_id, old_row->row_id());
      if (auto new_row = model_->FindRow(old_row->key())) {
        new_row->set_row_id(old_row->row_id());
        row_map_[new_row->row_id()] = new_row;
        if (*new_row != *old_row)
          observer->DidChangeRow(new_row->row_id());
      } else {
        row_map_.erase(row_map_.find(old_row->row_id()));
        observer->DidRemoveRow(old_row->row_id());
        removed_rows.push_back(old_row);
      }
    }
    for (auto new_row : model_->rows()) {
      if (new_row->row_id())
        continue;
      if (removed_rows.empty()) {
        ++max_row_id;
        new_row->set_row_id(max_row_id);
      } else {
        new_row->set_row_id(removed_rows.back()->row_id());
        removed_rows.pop_back();
      }
      row_map_[new_row->row_id()] = new_row;
      observer->DidAddRow(new_row->row_id());
    }
    return;
  }

  // Table layout is changed.
  model_ = std::move(new_model);
  if (control_) {
    control_->DestroyWidget();
    control_ = nullptr;
  }
  columns_ = std::move(BuildColumns(model_->header_row()));

  row_map_.clear();
  auto next_row_id = 1;
  for (auto row : model_->rows()) {
    if (!row->row_id()) {
      row->set_row_id(next_row_id);
      ++next_row_id;
    }
    row_map_[row->row_id()] = row;
  }

  // Adjust column width by contents
  for (auto row : model_->rows()) {
    auto column_runner = columns_.begin();
    for (auto& cell : row->cells()) {
      column_runner->width = std::max(column_runner->width, CellWidth(cell));
      ++column_runner;
    }
  }

  // Extend the right most column to fill window.
  auto width = 0.0f;
  for (auto& column : columns_) {
    width += column.width;
  }
  if (width < bounds().width())
    columns_.back().width = bounds().width() - width;

  control_ = new ui::TableControl(columns_, this, this);
  AppendChild(control_);
  control_->Realize(bounds());
  control_->Show();
}

std::unique_ptr<TableViewModel> TableView::UpdateModelIfNeeded() {
  if (!should_update_model_)
    return std::unique_ptr<TableViewModel>();
  UI_ASSERT_DOM_LOCKED();
  should_update_model_ = false;
  std::unique_ptr<TableViewModel> model(new TableViewModel());
  auto const buffer = document_->buffer();
  auto position = buffer->ComputeEndOfLine(0);
  auto header_line = buffer->GetText(0, position);
  model->SetHeaderRow(header_line);
  for (;;) {
    ++position;
    if (position == buffer->GetEnd())
      break;
    auto const start = position;
    position = buffer->ComputeEndOfLine(position);
    auto const line = buffer->GetText(start, position);
    model->AddRow(line);
  }
  return std::move(model);
}

// text::BufferMutationObserver
void TableView::DidDeleteAt(Posn, size_t) {
  should_update_model_ = true;
}

// ui::Animatable
void TableView::Animate(base::Time) {
  if (!visible())
    return;
  // TODO(eval1749) We don't need to schedule animation for each animation
  // frame for |TableView|.
  ui::Animator::instance()->ScheduleAnimation(this);
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  if (!lock_scope.locked())
    return;
  auto new_model = UpdateModelIfNeeded();
  if (new_model)
    UpdateControl(std::move(new_model));
  if (has_focus())
    control_->RequestFocus();
  control_->RenderIfNeeded(canvas_.get());
  DidAnimate();
}

void TableView::DidInsertAt(Posn, size_t) {
  should_update_model_ = true;
}

// ui::TableControlObserver
void TableView::OnKeyPressed(const ui::KeyboardEvent& event) {
  ContentWindow::OnKeyPressed(event);
}

void TableView::OnMousePressed(const ui::MouseEvent& event) {
  ContentWindow::OnMousePressed(event);
}

void TableView::OnSelectionChanged() {
}

// ui::TableModel
int TableView::GetRowCount() const {
  return static_cast<int>(model_->row_count());
}

int TableView::GetRowId(int index) const {
  const auto& row = model_->row(static_cast<size_t>(index));
  return row.row_id();
}

base::string16 TableView::GetCellText(int row_id, int column_id) const {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return base::string16();
  }
  return present->second->cell(static_cast<size_t>(column_id)).text();
}

// ui::Widget
// Resize |ui::TableControl| to cover all client area.
void TableView::DidChangeBounds() {
  ContentWindow::DidChangeBounds();
  if (canvas_)
    canvas_->SetBounds(GetContentsBounds());
  if (control_)
    control_->SetBounds(gfx::ToEnclosingRect(GetContentsBounds()));
}

void TableView::DidHide() {
  ContentWindow::DidHide();
  canvas_.reset();
}

void TableView::DidRealize() {
  ContentWindow::DidRealize();
  document_->buffer()->AddObserver(this);
}

void TableView::DidShow() {
  ContentWindow::DidShow();
  DCHECK(!canvas_);
  if (bounds().empty())
    return;
  canvas_.reset(layer()->CreateCanvas());
}

// views::ContentWindow
void TableView::MakeSelectionVisible() {
}

}  // namespace views
