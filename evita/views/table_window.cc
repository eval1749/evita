// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/table_window.h"

#include <algorithm>

#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/text/text_document.h"
#include "evita/editor/dom_lock.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/text/buffer.h"
#include "evita/ui/animation/animator.h"
#include "evita/ui/base/table_model_observer.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/controls/table_control.h"
#include "evita/views/icon_cache.h"
#include "evita/views/table_view_model.h"

extern HINSTANCE g_hInstance;

namespace views {

namespace {

float CellWidth(const TableViewModel::Cell& cell) {
  // TODO(eval1749): We should get character width from ListView control.
  const auto kCharWidth = 6.0f;
  const auto kLeftMargin = 5.0f;
  const auto kRightMargin = 5.0f;
  return cell.text().length() * kCharWidth + kLeftMargin + kRightMargin;
}

std::vector<ui::TableColumn> BuildColumns(const TableViewModel::Row* row) {
  // TODO(eval1749): We should specify minimum/maximum column width from model.
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
TableWindow::TableWindow(WindowId window_id, dom::TextDocument* document)
    : CanvasContentWindow(window_id),
      control_(nullptr),
      document_(document),
      model_(new TableViewModel()),
      should_update_model_(true) {
  UI_DOM_AUTO_LOCK_SCOPE();
  document_->buffer()->AddObserver(this);
}

TableWindow::~TableWindow() {
  UI_DOM_AUTO_LOCK_SCOPE();
  document_->buffer()->RemoveObserver(this);
}

std::vector<int> TableWindow::GetRowStates(
    const std::vector<base::string16>& keys) const {
  std::vector<int> states;
  for (auto key : keys) {
    auto const row = model_->FindRow(key);
    auto const state = row ? control_->GetRowState(row->row_id()) : 0;
    states.push_back(state);
  }
  return std::move(states);
}

void TableWindow::Paint() {
  TRACE_EVENT0("view", "TextWindow::Paint");
  gfx::Canvas::DrawingScope drawing_scope(canvas());
  if (canvas()->should_clear())
    canvas()->Clear(gfx::ColorF::White);
  OnDraw(canvas());
}

void TableWindow::Redraw() {
  UI_ASSERT_DOM_LOCKED();
  auto new_model = UpdateModelIfNeeded();
  if (!new_model)
    return;
  UpdateControl(std::move(new_model));
}

void TableWindow::UpdateControl(std::unique_ptr<TableViewModel> new_model) {
  if (*model_->header_row() == *new_model->header_row()) {
    auto old_model = std::move(model_);
    model_ = std::move(new_model);
    auto const observer = control_->GetTableModelObserver();
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
  for (auto& column : columns_)
    width += column.width;
  if (width < bounds().width())
    columns_.back().width = bounds().width() - width;

  control_ = new ui::TableControl(columns_, this, this);
  AppendChild(control_);
  control_->SetBounds(gfx::ToEnclosingRect(GetContentsBounds()));
  control_->RealizeWidget();
}

std::unique_ptr<TableViewModel> TableWindow::UpdateModelIfNeeded() {
  if (!should_update_model_)
    return std::unique_ptr<TableViewModel>();
  UI_ASSERT_DOM_LOCKED();
  should_update_model_ = false;
  std::unique_ptr<TableViewModel> model(new TableViewModel());
  auto const buffer = document_->buffer();
  auto position = buffer->ComputeEndOfLine(text::Offset());
  auto header_line = buffer->GetText(text::Offset(), position);
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
void TableWindow::DidDeleteAt(const text::StaticRange& range) {
  should_update_model_ = true;
}

void TableWindow::DidInsertBefore(const text::StaticRange& range) {
  should_update_model_ = true;
}

// ui::AnimationFrameHandler
void TableWindow::DidBeginAnimationFrame(const base::TimeTicks& now) {
  if (!visible())
    return;
  if (!canvas()->IsReady())
    return RequestAnimationFrame();
  TRACE_EVENT0("scheduler", "TableWindow::DidBeginAnimationFrame");
  UI_DOM_AUTO_TRY_LOCK_SCOPE(lock_scope);
  if (!lock_scope.locked()) {
    // We hope we can update frame in next animation frame.
    RequestAnimationFrame();
    return;
  }
  auto new_model = UpdateModelIfNeeded();
  if (new_model)
    UpdateControl(std::move(new_model));
  if (has_focus())
    control_->RequestFocus();

  Paint();
  NotifyUpdateContent();
}

// ui::TableControlObserver
void TableWindow::OnKeyPressed(const ui::KeyEvent& event) {
  CanvasContentWindow::OnKeyPressed(event);
}

void TableWindow::OnMousePressed(const ui::MouseEvent& event) {
  CanvasContentWindow::OnMousePressed(event);
}

void TableWindow::OnSelectionChanged() {}

// ui::TableModel
int TableWindow::GetRowCount() const {
  return static_cast<int>(model_->row_count());
}

int TableWindow::GetRowId(int index) const {
  const auto& row = model_->row(static_cast<size_t>(index));
  return row.row_id();
}

base::string16 TableWindow::GetCellText(int row_id, int column_id) const {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return base::string16();
  }
  return present->second->cell(static_cast<size_t>(column_id)).text();
}

// ui::Widget
void TableWindow::DidActivate() {
  RequestAnimationFrame();
}

void TableWindow::DidChangeBounds() {
  CanvasContentWindow::DidChangeBounds();
  if (!control_)
    return;
  // Make |ui::TableControl| to cover all client area.
  control_->SetBounds(gfx::ToEnclosingRect(GetContentsBounds()));
}

void TableWindow::DidSetFocus(ui::Widget* last_focused) {
  CanvasContentWindow::DidSetFocus(last_focused);
  // We'll move focus to |TableControl| in |OnDraw()|.
  RequestAnimationFrame();
}

// views::CanvasContentWindow
void TableWindow::MakeSelectionVisible() {}

}  // namespace views
