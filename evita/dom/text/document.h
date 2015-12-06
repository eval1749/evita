// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_TEXT_DOCUMENT_H_
#define EVITA_DOM_TEXT_DOCUMENT_H_

#include <memory>
#include <vector>

#include "base/strings/string16.h"
#include "base/time/time.h"
#include "evita/dom/events/event_target.h"
#include "evita/gc/member.h"
#include "evita/precomp.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/scriptable.h"

namespace text {
class Buffer;
struct LineAndColumn;
}

namespace dom {

class Range;
class RegularExpression;

//////////////////////////////////////////////////////////////////////
//
// Document
//
class Document final : public v8_glue::Scriptable<Document, EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(Document);

 public:
  ~Document() final;

  const text::Buffer* buffer() const { return buffer_.get(); }
  text::Buffer* buffer() { return buffer_.get(); }
  base::char16 charCodeAt(text::Posn position) const;
  text::Posn length() const;
  bool modified() const;
  void set_modified(bool new_modified);
  bool read_only() const;
  void set_read_only(bool read_only) const;
  // Returns spelling at |offset|.
  const base::string16& spelling_at(text::Posn offset) const;
  // Returns syntax at |offset|.
  const base::string16& syntax_at(text::Posn offset) const;
  v8::Handle<v8::Object> style_at(text::Posn position) const;

  bool CheckCanChange() const;
  void ClearUndo();
  void EndUndoGroup(const base::string16& name);
  text::LineAndColumn GetLineAndColumn(text::Posn offset) const;
  bool IsValidPosition(text::Posn position) const;
  v8::Handle<v8::Value> Match(RegularExpression* regexp, int start, int end);
  text::Posn Redo(text::Posn position);
  void RenameTo(const base::string16& new_name);
  void SetSpelling(text::Posn start, text::Posn end, int spelling);
  void SetSyntax(text::Posn start,
                 text::Posn end,
                 const base::string16& syntax);
  base::string16 Slice(int start, int end);
  base::string16 Slice(int start);
  void StartUndoGroup(const base::string16& name);
  text::Posn Undo(text::Posn position);

  // Implementation of Document static method
  static Document* NewDocument();

 private:
  Document();

  std::unique_ptr<text::Buffer> buffer_;

  DISALLOW_COPY_AND_ASSIGN(Document);
};

}  // namespace dom

#endif  // EVITA_DOM_TEXT_DOCUMENT_H_
