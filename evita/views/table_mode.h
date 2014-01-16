// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_view_table_model_h)
#define INCLUDE_view_table_model_h

#include <vector>
#include <unordered_map>

#include "base/basictypes.h"

namespace view {

class TableModel {
  public: class Cell {
    private: base::string16 text_;

    public: explicit Cell(const base::string16& text);
    public: Cell(Cell&& other);
    public: ~Cell();

    public: Cell& operator=(Cell&& other);

    public: const base::string16& text() const { return text_; }
  };

  public: class Row {
    private: std::vector<Cell> cells_;
    private: size_t hash_code_;

    public: Row(Row&& other);
    public: Row();
    public: ~Row();

    public: Row& operator=(Row&& other);

    public: std::vector<Cell>::const_iterator begin() const {
      return cells_.begin();
    }
    public: const base::string16& cell(size_t column) const {
      return cells_[column];
    }
    public: std::vector<Cell>::const_iterator end() const {
      return cells_.end();
    }
    public: size_t length() const { return cells_.size() }
    public: void AddCell(const base::string16& text);
  };

  private: Row header_row_;
  private: std::unordered_map<base::string16, Row> rows_;

  public: TableModel();
  public: ~TableMode();

  public: iterator begin() const;
  public: iterator end() const;
  public: const Row* header_row() const { return &header_row_; }
  public: size_t size() const { return rows_.size(); }

  public: void AddRow(const base::string16& line);
  public: const Row* FindRow(const base::string16& key) const;
  public: void Clear();
  private: Row ParseLine(const base::string16& line);
  public: void SetHeaderRow(const base::string16& line);

  DISALLOW_COPY_AND_ASSIGN(TableModel);
};

}  // namespace view

#endif //!defined(INCLUDE_view_table_model_h)
