// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/table_view.h"

#include <algorithm>

#include "evita/cm_CmdProc.h"
#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/editor/application.h"
#include "evita/ui/base/selection_model.h"
#include "evita/ui/base/table_model_observer.h"
#include "evita/ui/controls/table_control.h"
#include "evita/vi_Frame.h"
#include "evita/views/table_model.h"

extern HINSTANCE g_hInstance;

namespace Command {
uint32 TranslateKey(uint32 vkey_code);
}

namespace views {

namespace {

float CellWidth(const TableData::Cell& cell) {
  // TODO(yosi) We should get character width from ListView control.
  const auto kCharWidth = 6.0f;
  return (cell.text().length() + 2) * kCharWidth;
}

std::vector<ui::TableColumn> BuildColumns(const TableData::Row* row) {
  std::vector<ui::TableColumn> columns;
  for (auto cell : *row) {
    ui::TableColumn column;
    column.alignment = ui::TableColumn::Alignment::Left;
    column.text = cell.text();
    column.width = CellWidth(cell);
  }
  return std::move(columns);
}

void NotifyModelChanges(ui::TableModelObserver* observer,
                        const TableData* new_model,
                        const TableData* old_model) {
  bool changed = false;
  auto index = 0u;
  auto changed_index = 0u;
  while (index < new_model->size() && index < old_model->size()) {
    if (new_model->row(index) != old_model->row(index)) {
      if (!changed)
        changed_index = index;
    } else if (changed) {
      changed = false;
      observer->DidChangeItems(static_cast<int>(changed_index),
                               static_cast<int>(index - changed_index));
    }
    ++index;
  }

  if (changed) {
    observer->DidChangeItems(static_cast<int>(changed_index),
                             static_cast<int>(index - changed_index));
  }

  if (index == new_model->size()) {
    observer->DidRemoveItems(static_cast<int>(index),
                             static_cast<int>(old_model->size() - index));
    return;
  }

  if (index == old_model->size()) {
    observer->DidAddItems(static_cast<int>(index),
                          static_cast<int>(new_model->size() - index));
  }
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TableView
//
TableView::TableView(WindowId window_id, dom::Document* document)
    : CommandWindow_(window_id),
      document_(document),
      data_(new TableData()),
      modified_tick_(0) {
}

TableView::~TableView() {
}

std::unique_ptr<TableData> TableView::CreateModel() {
  std::unique_ptr<TableData> model(new TableData());
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

void TableView::GetRowStates(const std::vector<base::string16>& keys,
                             int* states) const {
  std::unordered_map<const TableData::Row*, int> row_index_map;
  auto state_index = 0;
  for (auto key : keys) {
    if (auto const row = data_->FindRow(key))
      row_index_map[row] = state_index;
    else
      DVLOG(0) << "No such row: " << key;
    ++state_index;
  }

  for (auto row_index = 0u; row_index < data_->size(); ++row_index) {
    auto row = data_->row(row_index);
    auto const present = row_index_map.find(&row);
    if (present == row_index_map.end())
      continue;
    states[present->second] = control_->IsSelected(
        static_cast<int>(row_index));
  }
}

void TableView::Redraw() {
  auto const modified_tick = document_->buffer()->GetModfTick();
  if (modified_tick_ == modified_tick)
    return;
  modified_tick_ = modified_tick;

  std::unique_ptr<TableData> new_data(CreateModel());

  if (*new_data->header_row() == *data_->header_row()) {
    NotifyModelChanges(control_.get(), new_data.get(), data_.get());
  } else {
    if (control_)
      RemoveChild(control_.get());
    columns_ = BuildColumns(new_data->header_row());
    control_.reset(new ui::TableControl(columns_, this, this));
    AppendChild(control_.get());
  }

  data_ = std::move(new_data);
}

// ui::TableControlObserver
void TableView::OnKeyDown(int) {
}

void TableView::OnSelectionChanged() {
}

// ui::TableModel
int TableView::GetRowCount() const {
  return static_cast<int>(data_->size());
}

const base::string16& TableView::GetCellText(int row, int column_id) const {
  return data_->row(static_cast<size_t>(row)).
      cell(static_cast<size_t>(column_id)).text();
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

void TableView::UpdateStatusBar() const {
  Frame::FindFrame(*this)->SetStatusBar(0, L"");
}

// widgets::Widget
void TableView::DidRealize() {
  CommandWindow_::DidRealize();
  Redraw();
}

void TableView::DidResize() {
  CommandWindow::DidResize();
  Redraw();
}

void TableView::DidSetFocus() {
  Redraw();
  CommandWindow::DidSetFocus();
}

bool TableView::OnIdle(uint32) {
  if (!is_shown())
    return false;
  Redraw();
  return true;
}

}  // namespace views
