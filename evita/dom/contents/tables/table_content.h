// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_dom_contents_table_content_h)
#define INCLUDE_evita_dom_contents_table_content_h

#include "evita/dom/contents/content.h"

#include <vector>

namespace dom {
namespace contents {

class TableRow;

class TableContent : public v8_glue::Scriptable<TableContent, Content> {
  DECLARE_SCRIPTABLE_OBJECT(TableContent);

  private: int change_count_;
  private: TableRow* header_row_;
  private: std::vector<TableRow*> rows_;

  public: TableContent(const base::string16& name);
  public: ~TableContent();

  public: int change_count() const { return change_count_; }
  public: TableRow* header_row() const { return header_row_; }
  public: size_t length() const { return rows_.size(); }
  public: TableRow* row(size_t index) const;

  public: TableRow* AddRow(TableRow* row);
  public: void DidChangeRow(TableRow* row);
  public: void RemoveRow(size_t index);

  DISALLOW_COPY_AND_ASSIGN(TableContent);
};

}  // namespace contents
}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_contents_table_content_h)
