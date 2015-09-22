// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/text/range.h"

#include "base/strings/stringprintf.h"
#include "evita/css/style_selector.h"
#include "evita/dom/converter.h"
#include "evita/dom/script_host.h"
#include "evita/dom/text/document.h"
#include "evita/text/buffer.h"
#include "evita/text/marker_set.h"
#include "evita/text/range.h"
#include "evita/text/spelling.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Range
//
Range::Range(Document* document, text::Posn start, text::Posn end)
    : Range(document, new text::Range(document->buffer(), start, end)) {}

Range::Range(Document* document, text::Range* range)
    : document_(document), range_(range) {}

Range::~Range() {}

bool Range::collapsed() const {
  return range_->start() == range_->end();
}

int Range::end() const {
  return range_->end();
}

int Range::start() const {
  return range_->start();
}

base::string16 Range::text() const {
  return std::move(range_->text());
}

void Range::set_end(int offset) {
  if (!document_->IsValidPosition(offset))
    return;
  range_->set_end(offset);
}

void Range::set_start(int offset) {
  if (!document_->IsValidPosition(offset))
    return;
  range_->set_start(offset);
}

void Range::set_text(const base::string16& text) {
  if (!document_->CheckCanChange())
    return;
  range_->set_text(text);
}

Range* Range::CollapseTo(Posn offset) {
  if (!document_->IsValidPosition(offset))
    return this;
  range_->SetRange(offset, offset);
  return this;
}

Range* Range::InsertBefore(const base::string16& text) {
  if (!document_->CheckCanChange())
    return this;
  document_->buffer()->InsertBefore(start(), text);
  return this;
}

Range* Range::NewRange(v8_glue::Either<Document*, Range*> document_or_range) {
  if (document_or_range.is_left)
    return NewRange(document_or_range, 0, 0);
  auto const range = document_or_range.right;
  return new Range(range->document(), range->start(), range->end());
}

Range* Range::NewRange(v8_glue::Either<Document*, Range*> document_or_range,
                       int offset) {
  return NewRange(document_or_range, offset, offset);
}

Range* Range::NewRange(v8_glue::Either<Document*, Range*> document_or_range,
                       int start,
                       int end) {
  if (document_or_range.is_left)
    return new Range(document_or_range.left, start, end);
  auto const range = document_or_range.right;
  return new Range(range->document(), start, end);
}

void Range::SetSpelling(int spelling_code) const {
  struct Local {
    static const common::AtomicString& MapToSpelling(int spelling_code) {
      switch (spelling_code) {
        case text::Spelling::None:
          return common::AtomicString::Empty();
        case text::Spelling::Corrected:
          return css::StyleSelector::normal();
        case text::Spelling::Misspelled:
          return css::StyleSelector::misspelled();
        case text::Spelling::BadGrammar:
          return css::StyleSelector::bad_grammar();
      }
      return common::AtomicString::Empty();
    }
  };
  if (collapsed()) {
    ScriptHost::instance()->ThrowError(
        "Can't set spelling for collapsed range.");
    return;
  }
  document_->buffer()->spelling_markers()->InsertMarker(
      range_->start(), range_->end(), Local::MapToSpelling(spelling_code));
}

void Range::SetSyntax(const base::string16& syntax) const {
  if (collapsed()) {
    ScriptHost::instance()->ThrowError("Can't set syntax for collapsed range.");
    return;
  }
  document_->buffer()->syntax_markers()->InsertMarker(
      range_->start(), range_->end(), common::AtomicString(syntax));
}

}  // namespace dom
