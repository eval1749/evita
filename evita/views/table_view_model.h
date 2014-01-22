// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_views_table_view_model_h)
#define INCLUDE_evita_views_table_view_model_h

#include <iterator>
#include <memory>
#include <vector>
#include <unordered_map>

#include "base/basictypes.h"
#include "base/strings/string16.h"

namespace views {

class TableViewModel {
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

    private: Cells cells_;
    private: size_t hash_code_;
    private: int row_id_;

    public: Row();
    public: ~Row();

    public: bool operator==(const Row& other) const;
    public: bool operator!=(const Row& other) const;

    public: const Cell& cell(size_t column) const { return cells_[column]; }
    public: const Cells& cells() const { return cells_; }
    public: size_t length() const { return cells_.size(); }
    public: const base::string16& key() const { return cell(0).text(); }
    public: int row_id() const { return row_id_; }
    public: void set_row_id(int row_id) { row_id_ = row_id; }

    public: void AddCell(const base::string16& text);
    public: void Clear();

    DISALLOW_COPY_AND_ASSIGN(Row);
  };

  public: typedef std::unordered_map<base::string16, Row*> RowMap;
  public: typedef std::vector<Row*> Rows;

  private: Row header_row_;
  private: RowMap row_map_;
  private: Rows rows_;

  public: TableViewModel();
  public: ~TableViewModel();

  public: const Row* header_row() const { return &header_row_; }
  public: const Row& row(size_t index) const { return *rows_[index]; }
  public: const Rows& rows() const { return rows_; }
  public: size_t row_count() const { return rows_.size(); }

  public: void AddRow(const base::string16& line);
  public: Row* FindRow(const base::string16& key) const;
  public: void Clear();
  private: std::unique_ptr<Row> ParseLine(const base::string16& line);
  public: void SetHeaderRow(const base::string16& line);

  DISALLOW_COPY_AND_ASSIGN(TableViewModel);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_table_view_model_h)
