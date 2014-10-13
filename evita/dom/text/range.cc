// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/text/range.h"

#include "evita/css/style_selector.h"
#include "evita/dom/converter.h"
#include "evita/dom/script_host.h"
#include "evita/dom/text/document.h"
#include "evita/dom/text/regexp.h"
#include "evita/text/buffer.h"
#include "evita/text/marker_set.h"
#include "evita/text/range.h"
#include "evita/text/spelling.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/either.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// RangeClass
//
class RangeClass : public v8_glue::WrapperInfo {
  public: RangeClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~RangeClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &RangeClass::NewRange);
  }

  private: static Range* NewRange(
      v8_glue::Either<Document*, Range*> either_document_or_range,
      v8_glue::Optional<int> opt_start, v8_glue::Optional<int> opt_end) {
    if (either_document_or_range.is_left) {
      auto const document = either_document_or_range.left;
      auto const start = opt_start.get(0);
      auto const end = opt_end.get(start);
      return new Range(document, start, end);
    }
    auto const range = either_document_or_range.right;
    auto const document = range->document();
    if (opt_start.is_supplied) {
      return new Range(document, opt_start.value,
                       opt_end.get(opt_start.value));
    }
    return new Range(document, range->start(), range->end());
  }

  private: virtual v8::Handle<v8::ObjectTemplate> SetupInstanceTemplate(
      v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ) override {
    gin::ObjectTemplateBuilder builder(isolate, templ);
    builder
        .SetProperty("collapsed", &Range::collapsed)
        .SetProperty("document", &Range::document)
        .SetProperty("end", &Range::end, &Range::set_end)
        .SetProperty("start", &Range::start, &Range::set_start)
        .SetProperty("text", &Range::text, &Range::set_text)
        .SetMethod("collapseTo", &Range::CollapseTo)
        .SetMethod("insertBefore", &Range::InsertBefore)
        .SetMethod("setSpelling", &Range::SetSpelling)
        .SetMethod("setStyle", &Range::SetStyle)
        .SetMethod("setSyntax", &Range::SetSyntax);
    return builder.Build();
  }

  DISALLOW_COPY_AND_ASSIGN(RangeClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Range
//
DEFINE_SCRIPTABLE_OBJECT(Range, RangeClass);

Range::Range(Document* document, text::Posn start, text::Posn end)
    : Range(document, new text::Range(document->buffer(), start, end)) {
}

Range::Range(Document* document, text::Range* range)
    : document_(document), range_(range) {
}

Range::~Range() {
}

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
    ScriptHost::instance()->ThrowError(
      "Can't set syntax for collapsed range.");
    return;
  }
  document_->buffer()->syntax_markers()->InsertMarker(
    range_->start(), range_->end(), common::AtomicString(syntax));
}

}  // namespace dom
