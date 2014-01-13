// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_contents_table_row_h)
#define INCLUDE_evita_dom_contents_table_row_h

#include <vector>

#include "evita/gc/member.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {
namespace contents {

class TableCell;
class TableContent;

//////////////////////////////////////////////////////////////////////
//
// TableRow
//
class TableRow : public v8_glue::Scriptable<TableRow> {
  DECLARE_SCRIPTABLE_OBJECT(TableRow);

  private: std::vector<TableCell*> cells_;
  private: gc::Member<TableContent> table_;

  public: TableRow();
  public: virtual ~TableRow();

  public: TableCell* cell(size_t index) const;
  public: void set_cell(size_t index, TableCell* cell);
  public: size_t length() const { return cells_.size(); }
  public: void set_table(TableContent* table) { table_ = table; }

  public: void AddCell(TableCell* cell);
  public: void DidChangeCell(TableCell* cell);
  public: void RemoveCell(TableCell* cell);

  DISALLOW_COPY_AND_ASSIGN(TableRow);
};

}  // namespace contents
}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_contents_table_row_h)
