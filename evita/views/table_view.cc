// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/table_view.h"

#include <commctrl.h>
#include <algorithm>

#include "evita/cm_CmdProc.h"
#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/editor/application.h"
#include "evita/vi_Frame.h"
#include "evita/views/table_model.h"

extern HINSTANCE g_hInstance;

namespace Command {
uint32 TranslateKey(uint32 vkey_code);
}

namespace views {

namespace {

//////////////////////////////////////////////////////////////////////
//
// TableContentBuilder
//
class TableContentBuilder {
  private: typedef TableModel::Row Row;

  private: std::vector<int> column_widths_;
  private: HWND list_view_;
  private: const TableModel* new_model_;
  private: const TableModel* old_model_;

  public: TableContentBuilder(HWND list_view,
                              const TableModel* new_model,
                              const TableModel* old_model);
  public: ~TableContentBuilder() = default;

  public: void Build();
  private: void BuildHeader();
  private: void BuildRows();

  private: static int CellWidth(const TableModel::Cell& cell);
  private: void UpdateListViewItem(int row_index, const Row* row);

  DISALLOW_COPY_AND_ASSIGN(TableContentBuilder);
};

int CellWith(const TableModel::Cell& cell) {
  // TODO(yosi) We should get character width from ListView control.
  const int kCharWidth = 6;
  return static_cast<int>((cell.text().length() + 2) * kCharWidth);
}

std::vector<ui::TableColumn> BuildColumn(const Row* row) {
  std::vector<ui::TableColumn> columns;
  for (auto cell : row.cells()) {
    ui::TableColumn column;
    column.alignment = ui::TableColumn::Alignment::Left;
    column.text = cell.text();
    column.width = CellWidth(cell);
  }
  return columns;
}

void NotifyModelChange(ui::TableModelObserver* observer,
                       const TableModel* new_model,
                       const TableModel* old_model) {
  auto new_index = 0u;
  auto old_index = 0u;
  while (new_index < new_model->length() && old_index < old_model->length()) {
    if (new_model->row(new_index) != old_model->row(old_index))
      break;
    ++new_index;
    ++old_index;
  }

  if (new_index == new_model->length()) {
    observer->DidRemoveItems(old_index, old_mode->length() - old_index);
    return;
  }

  if (old_index == old_model->length()) {
    observer->DidAddItems(new_index, new_mode->length() - new_index);
    return;
  }

  // TODO(yosi): We should detect add/remove rows.
  observer->DidChangeModel();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TableView
//
TableView::TableView(WindowId window_id, dom::Document* document)
    : CommandWindow_(window_id),
      document_(document),
      list_view_(nullptr),
      model_(new TableModel()),
      modified_tick_(0) {
}

TableView::~TableView() {
  DCHECK(!list_view_);
}

std::unique_ptr<TableModel> TableView::CreateModel() {
  std::unique_ptr<TableModel> model(new TableModel());
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
  DCHECK(list_view_);
  std::unordered_map<const TableModel::Row*, int> row_index_map;
  auto state_index = 0;
  for (auto key : keys) {
    if (auto const row = model_->FindRow(key))
      row_index_map[row] = state_index;
    else
      DVLOG(0) << "No such row: " << key;
    ++state_index;
  }
  auto const num_items = ListView_GetItemCount(list_view_);
  for (auto item_index = 0; item_index < num_items; ++item_index) {
    LVITEM item = {0};
    item.iItem = item_index;
    item.mask = LVIF_PARAM | LVIF_STATE;
    item.stateMask = LVIS_ACTIVATING | LVIS_CUT | LVIS_DROPHILITED |
                     LVIS_FOCUSED | LVIS_SELECTED;
    if (!ListView_GetItem(list_view_, &item)) {
      DVLOG(0) << "ListView_GetItem failed for iItem=" << item_index;
      continue;
    }
    auto const row = reinterpret_cast<TableModel::Row*>(item.lParam);
    auto const present = row_index_map.find(row);
    if (present != row_index_map.end())
      states[present->second] = static_cast<int>(item.state);
  }
}

void TableView::Redraw() {
  DCHECK(list_view_);

  auto const modified_tick = document_->buffer()->GetModfTick();
  if (modified_tick_ == modified_tick)
    return;
  modified_tick_ = modified_tick;

  std::unique_ptr<TableModel> new_model(CreateModel());

  if (*new_model->header_row() == *model_->header_row()) {
    NotifyModelChanges(control_, new_model, model_);
  } else {
    if (control_)
      RemoveChild(control_);
    columns_ = BuildColumns(new_model->header_row());
    control_ = new ui::TableControl(columns_, new_model, this);
    AppendChild(control_);
    
  }

  model_ = std::move(new_model);
}

// views::CommandWindow
Command::KeyBindEntry* TableView::MapKey(uint nKey) {
  if (auto const entry = document_->buffer()->MapKey(nKey))
    return entry;

  return CommandWindow::MapKey(nKey);
}

// views::ContentWindow
base::string16 TableView::GetTitle(size_t) const {
  return L"*buffer list*";
}

void TableView::MakeSelectionVisible() {
}



void TableView::UpdateStatusBar() const {
  Frame::FindFrame(*this)->SetStatusBar(0, L"");
}

// ui::TableControlObserver
void TableView::OnKeyDown(int) {
}

void TableView::OnSelectionChanged() {
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
  if (!list_view_)
    return;
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
