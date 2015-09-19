// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_VIEWS_TABLE_VIEW_MODEL_H_
#define EVITA_VIEWS_TABLE_VIEW_MODEL_H_

#include <iterator>
#include <memory>
#include <vector>
#include <unordered_map>

#include "base/basictypes.h"
#include "base/strings/string16.h"

namespace views {

class TableViewModel final {
 public:
  class Cell final {
   public:
    explicit Cell(const base::string16& text);
    Cell(Cell&& other);
    ~Cell();

    Cell& operator=(Cell&& other);
    bool operator==(const Cell& other) const;
    bool operator!=(const Cell& other) const;

    const base::string16& text() const { return text_; }

   private:
    base::string16 text_;
  };

  class Row final {
   public:
    using Cells = std::vector<Cell>;

    Row();
    ~Row();

    bool operator==(const Row& other) const;
    bool operator!=(const Row& other) const;

    const Cell& cell(size_t column) const { return cells_[column]; }
    const Cells& cells() const { return cells_; }
    size_t length() const { return cells_.size(); }
    const base::string16& key() const { return cell(0).text(); }
    int row_id() const { return row_id_; }
    void set_row_id(int row_id) { row_id_ = row_id; }

    void AddCell(const base::string16& text);
    void Clear();

   private:
    Cells cells_;
    size_t hash_code_;
    int row_id_;

    DISALLOW_COPY_AND_ASSIGN(Row);
  };

  typedef std::unordered_map<base::string16, Row*> RowMap;
  typedef std::vector<Row*> Rows;

  TableViewModel();
  ~TableViewModel();

  const Row* header_row() const { return &header_row_; }
  const Row& row(size_t index) const { return *rows_[index]; }
  const Rows& rows() const { return rows_; }
  size_t row_count() const { return rows_.size(); }

  void AddRow(const base::string16& line);
  Row* FindRow(const base::string16& key) const;
  void Clear();
  void SetHeaderRow(const base::string16& line);

 private:
  std::unique_ptr<Row> ParseLine(const base::string16& line);

  Row header_row_;
  RowMap row_map_;
  Rows rows_;

  DISALLOW_COPY_AND_ASSIGN(TableViewModel);
};

}  // namespace views

#endif  // EVITA_VIEWS_TABLE_VIEW_MODEL_H_
