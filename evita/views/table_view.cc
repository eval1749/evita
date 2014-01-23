// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/table_view.h"

#include <commctrl.h>
#include <algorithm>

#include "evita/cm_CmdProc.h"
#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/ui/base/table_model_observer.h"
#include "evita/ui/controls/table_control.h"
#include "evita/vi_Frame.h"
#include "evita/views/table_view_model.h"

extern HINSTANCE g_hInstance;

namespace Command {
uint32 TranslateKey(uint32 vkey_code);
}

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
  std::vector<ui::TableColumn> columns;
  for (auto& cell : row->cells()) {
    ui::TableColumn column;
    column.alignment = ui::TableColumn::Alignment::Left;
    column.column_id = static_cast<int>(columns.size());
    column.text = cell.text();
    column.width = CellWidth(cell);
    columns.push_back(column);
  }
  return std::move(columns);
}

void NotifyModelChanges(ui::TableModelObserver* observer,
                        const TableViewModel* new_model,
                        const TableViewModel* old_model) {
  std::vector<TableViewModel::Row*> removed_rows;
  auto max_row_id = 0;
  for (auto old_row : old_model->rows()) {
    max_row_id = std::max(max_row_id, old_row->row_id());
    if (auto new_row = new_model->FindRow(old_row->key())) {
      new_row->set_row_id(old_row->row_id());
      if (*new_row != *old_row)
        observer->DidChangeRow(new_row->row_id());
    } else {
      observer->DidRemoveRow(old_row->row_id());
      removed_rows.push_back(old_row);
    }
  }
  for (auto new_row : new_model->rows()) {
    if (new_row->row_id())
      continue;
    if (removed_rows.empty()) {
      ++max_row_id;
      new_row->set_row_id(max_row_id);
    } else {
      new_row->set_row_id(removed_rows.back()->row_id());
      removed_rows.pop_back();
    }
    observer->DidAddRow(new_row->row_id());
  }
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TableView
//
TableView::TableView(WindowId window_id, dom::Document* document)
    : CommandWindow_(window_id),
      control_(nullptr),
      document_(document),
      model_(new TableViewModel()),
      modified_tick_(0) {
}

TableView::~TableView() {
}

std::unique_ptr<TableViewModel> TableView::CreateModel() {
  UI_DOM_AUTO_LOCK_SCOPE();
  std::unique_ptr<TableViewModel> model(new TableViewModel());
  auto const buffer = document_->buffer();
  auto position = buffer->ComputeEndOf(Unit_Paragraph, 0);
  auto header_line = buffer->GetText(0, position);
  model->SetHeaderRow(header_line);
  for (;;) {
    ++position;
    if (position == buffer->GetEnd())
      break;
    auto const start = position;
    position = buffer->ComputeEndOf(Unit_Paragraph, position);
    auto const line = buffer->GetText(start, position);
    model->AddRow(line);
  }
  return std::move(model);
}

bool TableView::DrawIfNeeded() {
  auto const modified_tick = document_->buffer()->GetModfTick();
  if (modified_tick_ == modified_tick)
    return false;
  modified_tick_ = modified_tick;

  std::unique_ptr<TableViewModel> new_data(CreateModel());

  if (*new_data->header_row() == *model_->header_row()) {
    NotifyModelChanges(control_, new_data.get(), model_.get());
  } else {
    if (control_) {
      control_->DestroyWidget();
      control_ = nullptr;
    }
    columns_ = std::move(BuildColumns(new_data->header_row()));
    auto row_id = 0;
    for (auto row : new_data->rows()) {
      ++row_id;
      row->set_row_id(row_id);
    }
  }

  rows_.clear();
  row_map_.clear();
  for (auto row : new_data->rows()) {
    rows_.push_back(row);
    DCHECK(row->row_id());
    row_map_[row->row_id()] = row;
  }

  model_ = std::move(new_data);
  if (control_)
    return true;

  for (auto row : model_->rows()) {
    auto column_runner = columns_.begin();
    for (auto& cell : row->cells()) {
      column_runner->width = std::max(column_runner->width, CellWidth(cell));
      ++column_runner;
    }
  }
  control_ = new ui::TableControl(columns_, this, this);
  AppendChild(control_);
  control_->Realize(rect());
  return true;
}

void TableView::GetRowStates(const std::vector<base::string16>& keys,
                             int* states) const {
  std::unordered_map<const TableViewModel::Row*, int> row_index_map;
  auto state_index = 0;
  for (auto key : keys) {
    if (auto const row = model_->FindRow(key))
      row_index_map[row] = state_index;
    else
      DVLOG(0) << "No such row: " << key;
    ++state_index;
  }

  for (auto pair : row_index_map) {
    states[pair.second] = control_->IsSelected(pair.first->row_id());
  }
}

// ui::TableControlObserver
void TableView::OnKeyDown(int key_code) {
  Application::instance()->Execute(this, static_cast<uint32_t>(key_code), 0u);
}

void TableView::OnSelectionChanged() {
}

// ui::TableModel
int TableView::GetRowCount() const {
  return static_cast<int>(model_->row_count());
}

int TableView::GetRowId(int index) const {
  return rows_[static_cast<size_t>(index)]->row_id();
}

base::string16 TableView::GetCellText(int row_id, int column_id) const {
  auto const present = row_map_.find(row_id);
  if (present == row_map_.end()) {
    DVLOG(0) << "No such row " << row_id;
    return base::string16();
  }
  return present->second->cell(static_cast<size_t>(column_id)).text();
}

// views::CommandWindow
Command::KeyBindEntry* TableView::MapKey(uint nKey) {
  if (auto const entry = document_->buffer()->MapKey(nKey))
    return entry;

  return CommandWindow::MapKey(nKey);
}

// views::ContentWindow
base::string16 TableView::GetTitle(size_t) const {
  return L"*document list*";
}

void TableView::MakeSelectionVisible() {
}

void TableView::Redraw() {
  DrawIfNeeded();
}

void TableView::UpdateStatusBar() const {
  Frame::FindFrame(*this)->SetStatusBar(0, L"");
}

// ui::Widget
void TableView::DidRealize() {
  CommandWindow_::DidRealize();
  Redraw();
}

void TableView::DidResize() {
  CommandWindow::DidResize();
  control_->ResizeTo(rect());
}

void TableView::DidSetFocus() {
  Redraw();
  CommandWindow::DidSetFocus();
  control_->SetFocus();
}

bool TableView::OnIdle(uint32) {
  if (!is_shown())
    return false;
  return DrawIfNeeded();
}

void TableView::Show() {
  BaseWindow::Show();
  Redraw();
}

}  // namespace views
