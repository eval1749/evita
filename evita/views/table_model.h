// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_views_table_model_h)
#define INCLUDE_evita_views_table_model_h

#include <iterator>
#include <memory>
#include <vector>
#include <unordered_map>

#include "base/basictypes.h"
#include "base/strings/string16.h"

namespace views {

class TableData {
  public: class Cell {
    private: base::string16 text_;

    public: explicit Cell(const base::string16& text);
    public: Cell(Cell&& other);
    public: ~Cell();

    public: Cell& operator=(Cell&& other);
    public: bool operator==(const Cell& other) const;
    public: bool operator!=(const Cell& other) const;

    public: const base::string16& text() const { return text_; }
  };

  public: class Row {
    private: typedef std::vector<Cell> Cells;
    public: typedef Cells::const_iterator iterator;

    private: Cells cells_;
    private: size_t hash_code_;

    public: Row(Row&& other);
    public: Row();
    public: ~Row();

    public: Row& operator=(Row&& other);
    public: bool operator==(const Row& other) const;
    public: bool operator!=(const Row& other) const;

    public: iterator begin() const { return cells_.begin(); }
    public: const Cell& cell(size_t column) const { return cells_[column]; }
    public: iterator end() const { return cells_.end(); }
    public: const base::string16& key() const { return cell(0).text(); }
    public: size_t length() const { return cells_.size(); }

    public: void AddCell(const base::string16& text);
    public: void Clear();

    DISALLOW_COPY_AND_ASSIGN(Row);
  };

  public: typedef std::unordered_map<base::string16, Row*> RowMap;
  public: typedef std::vector<Row*> Rows;

  private: Row header_row_;
  private: RowMap row_map_;
  private: Rows rows_;

  public: TableData();
  public: ~TableData();

  public: const Row* header_row() const { return &header_row_; }
  public: const Row& row(size_t index) const { return *rows_[index]; }
  public: const Rows& rows() const { return rows_; }
  public: size_t size() const { return rows_.size(); }

  public: void AddRow(const base::string16& line);
  public: const Row* FindRow(const base::string16& key) const;
  public: void Clear();
  private: std::unique_ptr<Row> ParseLine(const base::string16& line);
  public: void SetHeaderRow(const base::string16& line);

  DISALLOW_COPY_AND_ASSIGN(TableData);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_table_model_h)
