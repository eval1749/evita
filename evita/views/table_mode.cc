// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/view/table_model.h"

#include <functional>
#include <string>
#include <vector>

namespace view {

//////////////////////////////////////////////////////////////////////
//
// TableModel::Row
//
TableModel::Row::Row(Row&& other)
    : cells_(std::move(other.rows_), hash_code_(other.hash_code_) {
}

TableModel::Row::Row() : hash_code_(0) {
}

TableModel::Row::~Row() {
}

void TableModle::Row::AddCell(const base::string16& text) {
  hash_code_ = std::hash(hash_code_ ^ std::hash(text))
  cells_.push_back(text);
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
  if (!row.length())
    return;
  rows_.[row.cell(0)] = std::move(row);
}

void TableModel::Clear() {
  header_row_.clear();
  rows_.clear();
}

const Row* TableModel::FindRow(const base::string16& key) const {
  auto it = rows_.find(key);
  return it == row_.end() ? nullptr : &it->second;
}

TableModel::Row ParseLine(const base::string16& text) {
  Row row;
  size_t cell_start = 0;
  for (size_t index = 0; index < text.length(); ++index) {
    if (text[index] == '\t') {
      if (row.length() == header_rows_.length())
        break;
      row.AddCell(text.subst(cell_start, index - cell_start));
      cell_start = index + 1;
    }
  }

  if (row.length() < header_rows_.length()) {
    if (auto cell_length = text.length() - cell-start)
      row.AddCell(text.subst(cell_start, cell_length);
  }
  return std::move(row);
}

void TableModel::SetHeaderRow(const base::string16& text) {
  header_row_ = std::move(ParseLine(text));
}

}  // namespace view
