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

TableContentBuilder::TableContentBuilder(HWND list_view,
                                         const TableModel* new_model,
                                         const TableModel* old_model)
    : list_view_(list_view), new_model_(new_model), old_model_(old_model) {
}

void TableContentBuilder::Build() {
  BuildHeader();
  BuildRows();

  for (auto index = 0u; index < column_widths_.size(); ++index) {
    ListView_SetColumnWidth(list_view_, index, column_widths_[index]);
  }
}

void TableContentBuilder::BuildHeader() {
  LVCOLUMN column = {0};
  column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  column_widths_.clear();

  auto const new_num_cols = new_model_->header_row()->length();
  auto const old_num_cols = old_model_->header_row()->length();
  for (auto col_index = 0u; col_index < old_num_cols; ++col_index) {
    if (col_index >= new_num_cols) {
      ListView_DeleteColumn(list_view_, col_index);
      continue;
    }
    auto const new_cell = new_model_->header_row()->cell(col_index);
    auto const old_cell = old_model_->header_row()->cell(col_index);
    auto const new_cell_width = CellWidth(new_cell);
    column_widths_.push_back(new_cell_width);
    if (new_cell == old_cell)
      continue;
    column.cx = new_cell_width;
    column.fmt = LVCFMT_LEFT;
    column.pszText = const_cast<base::char16*>(new_cell.text().c_str());
    column.iSubItem = static_cast<int>(col_index);
    ListView_SetColumn(list_view_, col_index, &column);
  }

  for (auto index = old_num_cols; index < new_num_cols; ++index) {
    auto const new_cell = new_model_->header_row()->cell(index);
    auto const new_cell_width = CellWidth(new_cell);
    column_widths_.push_back(new_cell_width);
    column.cx = new_cell_width;
    column.fmt = LVCFMT_LEFT;
    column.pszText = const_cast<base::char16*>(new_cell.text().c_str());
    column.iSubItem = static_cast<int>(index);
    ListView_InsertColumn(list_view_, index, &column);
  }
}

void TableContentBuilder::BuildRows() {
  std::unordered_set<base::string16> present_set;

  std::vector<int> old_item_indices;

  auto const num_items = ListView_GetItemCount(list_view_);
  for (auto item_index = 0; item_index < num_items; ++item_index) {
    LVITEM item = {0};
    item.iItem = item_index;
    item.mask = LVIF_PARAM;
    if (!ListView_GetItem(list_view_, &item)) {
      DVLOG(0) << "Why do we fail on ListView_GetItem()?";
      old_item_indices.push_back(item_index);
      continue;
    }
    auto const old_row = reinterpret_cast<const Row*>(item.lParam);
    auto const new_row = new_model_->FindRow(old_row->key());
    if (!new_row) {
      old_item_indices.push_back(item_index);
      continue;
    }

    present_set.insert(old_row->key());
    UpdateListViewItem(item_index, new_row);
  }

  while (old_item_indices.size()) {
    ListView_DeleteItem(list_view_, old_item_indices.back());
    old_item_indices.pop_back();
  }

  for (auto new_row : new_model_->rows()) {
    if (present_set.find(new_row->key()) != present_set.end())
      continue;
    LVITEM item = {0};
    item.mask = LVIF_TEXT;
    item.pszText = const_cast<base::char16*>(new_row->key().c_str());
    auto const row_index = ListView_InsertItem(list_view_, &item);
    DCHECK_GE(row_index, 0);
    UpdateListViewItem(row_index, new_row);
  }
}

int TableContentBuilder::CellWidth(const TableModel::Cell& cell) {
  // TODO(yosi) We should get character width from ListView control.
  const int kCharWidth = 6;
  return static_cast<int>((cell.text().length() + 2) * kCharWidth);
}

void TableContentBuilder::UpdateListViewItem(int row_index, const Row* row) {
  LVITEM item = {0};
  item.lParam = reinterpret_cast<LPARAM>(row);
  item.mask = LVIF_PARAM;
  ListView_SetItem(list_view_, &item);

  column_widths_[0] = std::max(column_widths_[0], CellWidth(row->cell(0)));
  for (auto index = 1u; index < row->length(); ++index) {
    auto cell = row->cell(index);
    LVITEM item = {0};
    item.iItem = row_index;
    item.iSubItem = static_cast<int>(index);
    item.mask = LVIF_TEXT;
    item.pszText = const_cast<base::char16*>(cell.text().c_str());
    ListView_SetItem(list_view_, &item);
    column_widths_[index] = std::max(column_widths_[index], CellWidth(cell));
  }
}

int CALLBACK CompareItems(LPARAM a, LPARAM b, LPARAM) {
  auto const row_a = reinterpret_cast<const TableModel::Row*>(a);
  auto const row_b = reinterpret_cast<const TableModel::Row*>(b);
  return row_a->key().compare(row_b->key());
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

  TableContentBuilder builder(list_view_, new_model.get(), model_.get());
  builder.Build();

  ListView_SortItems(list_view_, CompareItems, nullptr);

  if (!ListView_GetSelectedCount(list_view_)) {
    // TODO(yosi) We should select the last active window rather than the
    // first // one.
    auto const state = LVIS_SELECTED | LVIS_FOCUSED;
    ListView_SetItemState(list_view_, 0, state, state);
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

// widgets::Widget
void TableView::DidRealize() {
  CommandWindow_::DidRealize();
  DCHECK(!list_view_);
  auto const dwExStyle = LVS_EX_DOUBLEBUFFER |
                         LVS_EX_FULLROWSELECT |
                         // LVS_EX_GRIDLINES |
                        LVS_EX_HEADERDRAGDROP |
                        LVS_EX_LABELTIP |
                        LVS_EX_UNDERLINEHOT;
                        // |= LVS_EX_TRACKSELECT;

  auto const dwStyle = WS_CHILD | WS_VISIBLE | LVS_REPORT |
                       LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS;

  list_view_ = ::CreateWindowEx(0, //dwExStyle
                                WC_LISTVIEW,
                                nullptr,           // title
                                dwStyle,
                                rect().left, rect().top,
                                rect().width(), rect().height(),
                                AssociatedHwnd(),
                                reinterpret_cast<HMENU>(this),
                                g_hInstance,
                                nullptr);

  // TODO(yosi) We should set image list for icon in ListView.
  ListView_SetExtendedListViewStyleEx(list_view_, dwExStyle, dwExStyle);
  Redraw();
}

void TableView::DidResize() {
  CommandWindow::DidResize();
  DCHECK(list_view_);
  ::SetWindowPos(list_view_, nullptr, rect().left, rect().top,
                 rect().width(), rect().height(),
                 SWP_NOZORDER);
}

void TableView::DidSetFocus() {
  if (!list_view_)
    return;
  ::SetFocus(list_view_);
  Redraw();
  CommandWindow::DidSetFocus();
}

void TableView::Hide() {
  ::ShowWindow(list_view_, SW_HIDE);
  BaseWindow::Hide();
}

bool TableView::OnIdle(uint32) {
  if (!is_shown())
    return false;
  Redraw();
  return true;
}

LRESULT TableView::OnNotify(NMHDR* nmhdr) {
  if (nmhdr->code == LVN_KEYDOWN) {
    auto const vkey_code = reinterpret_cast<NMLVKEYDOWN*>(nmhdr)->wVKey;
    auto const key_code = Command::TranslateKey(vkey_code);
    if (!key_code)
      return FALSE;
    if (MapKey(key_code)) {
      Application::instance()->Execute(this, key_code, 0);
      return TRUE;
    }
  }
  return FALSE;
}

void TableView::Show() {
  BaseWindow::Show();
  ::ShowWindow(list_view_, SW_SHOW);
}

void TableView::WillDestroyWidget() {
  auto list_view = list_view_;
  list_view_ = nullptr;
  ::DestroyWindow(list_view);
  CommandWindow::WillDestroyWidget();
}

}  // namespace views
