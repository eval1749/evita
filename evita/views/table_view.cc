// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/view/table_view.h"

#include <commctrl.h>

#include "evita/view/table_model.h"

namespace view {

namespace {
const int kListViewId = 1234;
}

TableView::TableView(dom::Document* document)
    : document_(document),
      list_view_(nullptr),
      model_(new TableModel()) {
}

TableView::~TableView() {
  DCHECK(!list_view_);
}

void TableView::Redraw() {
  DCHECK(list_view_);

  std::unique_ptr<TableModel> model(CreateModel());

  const auto char_width = 20;
  LVCOLUMN column = {0};
  column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

  // TODO(yosi) update column width
  for (auto index = 0; index < model_->header_row().size(); ++index) {
    if (index >= model->header_row()->size()) {
      ListView_DeleteColumn(list_view_, index);
      continue;
    }
    auto const cell = model->header_row()->cell(column_index);
    auto const old_cell = model_->header_row()->cell(column_index);
    if (cell == old_cell)
      continue;
    column.cx = (cell.text().length() + 2) * char_width;
    column.fmt = LVCFMT_LEFT;
    column.pszText = const_cast<base::char16>(cell.text().c_str());
    column.iSubitem = index;
    ListView_SetColumn(list_view_, index, &column)
  }

  for (auto index = model_->header_row()->size();
       index < model->header_row()->size();
       ++index) {
    column.cx = (cell.text().length() + 2) * char_width;
    column.fmt = LVCFMT_LEFT;
    column.pszText = const_cast<base::char16>(cell.text().c_str());
    column.iSubitem = index;
    ListView_InsertColumn(list_view_, index, &column)
  }


  auto column_index = 0;
  for (auto cell : model->header_row()) {
    column.cx = (cell.text().length() + 2) * char_width;
    column.fmt = LVCFMT_LEFT;
    column.pszText = const_cast<base::char16>(cell.text().c_str());
    column.iSubitem = column_index;
    ListView_InsertColumn(list_view_, column_index, &column);
    ++column_index;
  }

  std::unordered_set<const Row*> present_set;
  for (auto index = 0; index < model_->size(); ++index) {
    LVITEM item = {0};
    item.iItem = index;
    item.mask = LVIF_PARAM;
    ListView_GetItem(list_view_, &item);
    auto row = reinterpret_cast<const Row*>(item.lParam);
    if (model->FindRow(row.cell(0)))
      present_set.insert(row);
    else
      ListView_DeleteItem(list_view_, index);
  }

  for (auto row : model) {
    if (present_set.find(row) != present_set.end())
      continue;
    LVIEM item = {0};
    item.lParam = reinterpret_cast<LPARAM>(row);
    item.mask = LVIF_PARAM | LVIF_TEXT;
    item.pszText = const_cast<base::char16*>(row->cell(0).c_str());
    LiveView_InsertItem(list_view_, &item);

    for (auto index = 1; index < row.length(); ++index) {
      auto cell = row.cell(index);
      LVIEM item = {0};
      item.iSubitem = index;
      item.pszText = const_cast<base::char16*>(cell.c_str());
      ListView_SetItem(list_view_, &item);
    }
  }

  ListView_SortItems(list_view_, CompareItems, nullptr);

  // TODO(yosi) Keep selection.
  // TODO(yosi) We should select the last active window rather than the first
  // one.
  auto const state = LVIS_SELECTED | LVIS_FOCUSED;
  ListView_SetItemState(list_view_, 0, state, state);

  model_ = std::move(model);
}

void TableView::DidRealize() {
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
                                0, 0,
                                rect().width(), rect().height(),
                                *native_window(),
                                reinterpret_cast<HMENU>(kListViewId),
                                g_hInstance,
                                nullptr);

  ListView_SetExtendedListViewStyleEx(list_view_, dwExStyle, dwExStyle);
  ListView_SetImageList(list_view_, Application::instance()->GetIconList(),
                        LVSIL_SMALL);
  Redraw();
}

void TableView::DidResize() {
  DCHECK(list_view_);
  ::SetWindowPos(list_view_, nullptr, 0, 0,
                 rect().width(), rect().height(),
                 SWP_NOZORDER | SWP_SHOWWINDOW);
}

void TableView::DidSetFocus() {
  ::SetFocus(list_view_);
}

void TableView::Hide() {
  ::ShowWindow(list_view_, SW_HIDE);
  BaseWindow::Hide();
}

void TableView::Show() {
  BaseWindow::Show();
  ::ShowWindow(list_view_, SW_SHOW);
}

void TableView::WillDestroyWidget() {
  auto list_view = list_view_;
  list_view_ = nullptr;
  ::DestroyWindow(list_view);
}

}  // namespace view
