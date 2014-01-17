// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/table_model.h"

#include <functional>
#include <string>
#include <vector>

namespace view {

//////////////////////////////////////////////////////////////////////
//
// TableModel::Cell
//
TableModel::Cell::Cell(Cell&& other) : text_(std::move(other.text_)) {
}

TableModel::Cell::Cell(const base::string16& text) : text_(text) {
}

TableModel::Cell::~Cell() {
}

bool TableModel::Cell::operator==(const Cell& other) const {
  return text_ == other.text_;
}

bool TableModel::Cell::operator!=(const Cell& other) const {
  return text_ == other.text_;
}

//////////////////////////////////////////////////////////////////////
//
// TableModel::Row
//
TableModel::Row::Row(Row&& other)
    : cells_(std::move(other.cells_)), hash_code_(other.hash_code_) {
}

TableModel::Row::Row() : hash_code_(0) {
}

TableModel::Row::~Row() {
}

void TableModel::Row::AddCell(const base::string16& text) {
  std::hash<base::string16> hasher;
  hash_code_ = hash_code_ ^ hasher(text);
  cells_.push_back(std::move(Cell(text)));
}

void TableModel::Row::Clear() {
  cells_.clear();
}

//////////////////////////////////////////////////////////////////////
//
// TableModle
//
TableModel::TableModel() {
}

TableModel::~TableModel() {
}

void TableModel::AddRow(const base::string16& text) {
  auto row = std::move(ParseLine(text));
  if (!row->length())
    return;
  row_map_[row->key()] = row.get();
  rows_.push_back(row.release());
}

void TableModel::Clear() {
  for (auto row : rows_) {
    delete row;
  }
  header_row_.Clear();
  row_map_.clear();
  rows_.clear();
}

const TableModel::Row* TableModel::FindRow(const base::string16& key) const {
  auto it = row_map_.find(key);
  return it == row_map_.end() ? nullptr : it->second;
}

std::unique_ptr<TableModel::Row> TableModel::ParseLine(
    const base::string16& text) {
  std::unique_ptr<Row> row(new Row());
  size_t cell_start = 0;
  for (size_t index = 0; index < text.length(); ++index) {
    if (text[index] == '\t') {
      if (row->length() == header_row_.length())
        break;
      row->AddCell(text.substr(cell_start, index - cell_start));
      cell_start = index + 1;
    }
  }

  if (auto cell_length = text.length() - cell_start) {
    if (row->length() < header_row_.length())
      row->AddCell(text.substr(cell_start, cell_length));
  }

  while (row->length() < header_row_.length()) {
    row->AddCell(L"");
  }
  return std::move(row);
}

void TableModel::SetHeaderRow(const base::string16& text) {
  header_row_.Clear();
  size_t cell_start = 0;
  for (size_t index = 0; index < text.length(); ++index) {
    if (text[index] == '\t') {
      header_row_.AddCell(text.substr(cell_start, index - cell_start));
      cell_start = index + 1;
    }
  }

  if (auto cell_length = text.length() - cell_start)
    header_row_.AddCell(text.substr(cell_start, cell_length));
}

}  // namespace view
