// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_contents_table_cell_h)
#define INCLUDE_evita_dom_contents_table_cell_h

#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {
namespace contents {

class TableRow;

//////////////////////////////////////////////////////////////////////
//
// TableEow
//
class TableCell : public v8_glue::Scriptable<TableCell> {
  DECLARE_SCRIPTABLE_OBJECT(TableCell);

  private: gc::Member<TableRow> row_;
  private: base::string16 text_;

  public: TableCell(const base::string16& text);
  public: virtual ~TableCell();

  public: TableRow* row() const { return row_; }
  public: void set_row(TableRow* row) { row_ = row; }
  public: const base::string16& text() const { return text_; }
  public: void set_text(const base::string16& text);

  DISALLOW_COPY_AND_ASSIGN(TableCell);
};

} // namespace contents
} // namespace dom

#endif //!defined(INCLUDE_evita_dom_contents_table_cell_h)
