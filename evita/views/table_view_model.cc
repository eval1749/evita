// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/table_view_model.h"

#include <functional>
#include <string>
#include <vector>

namespace views {

//////////////////////////////////////////////////////////////////////
//
// TableViewModel::Cell
//
TableViewModel::Cell::Cell(Cell&& other) : text_(std::move(other.text_)) {}

TableViewModel::Cell::Cell(const base::string16& text) : text_(text) {}

TableViewModel::Cell::~Cell() {}

bool TableViewModel::Cell::operator==(const Cell& other) const {
  return text_ == other.text_;
}

bool TableViewModel::Cell::operator!=(const Cell& other) const {
  return text_ != other.text_;
}

//////////////////////////////////////////////////////////////////////
//
// TableViewModel::Row
//
TableViewModel::Row::Row() : hash_code_(0), row_id_(0) {}

TableViewModel::Row::~Row() {}

bool TableViewModel::Row::operator==(const Row& other) const {
  if (hash_code_ != other.hash_code_)
    return false;
  if (cells_.size() != other.cells_.size())
    return false;
  auto other_runner = other.cells_.begin();
  for (const auto& cell : cells_) {
    if (cell != *other_runner)
      return false;
    ++other_runner;
  }
  return true;
}

bool TableViewModel::Row::operator!=(const Row& other) const {
  return !operator==(other);
}

void TableViewModel::Row::AddCell(const base::string16& text) {
  std::hash<base::string16> hasher;
  hash_code_ = hash_code_ ^ hasher(text);
  cells_.push_back(std::move(Cell(text)));
}

void TableViewModel::Row::Clear() {
  cells_.clear();
}

//////////////////////////////////////////////////////////////////////
//
// TableModle
//
TableViewModel::TableViewModel() {}

TableViewModel::~TableViewModel() {
  Clear();
}

void TableViewModel::AddRow(const base::string16& text) {
  auto row = std::move(ParseLine(text));
  if (!row->length())
    return;
  row_map_[row->key()] = row.get();
  rows_.push_back(row.release());
}

void TableViewModel::Clear() {
  for (auto row : rows_) {
    delete row;
  }
  header_row_.Clear();
  row_map_.clear();
  rows_.clear();
}

TableViewModel::Row* TableViewModel::FindRow(const base::string16& key) const {
  auto it = row_map_.find(key);
  return it == row_map_.end() ? nullptr : it->second;
}

std::unique_ptr<TableViewModel::Row> TableViewModel::ParseLine(
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

void TableViewModel::SetHeaderRow(const base::string16& text) {
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

}  // namespace views
