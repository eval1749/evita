// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/range.h"

#include "evita/dom/buffer.h"
#include "evita/dom/converter.h"
#include "evita/dom/document.h"
#include "evita/dom/regexp.h"
#include "evita/text/range.h"
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
      auto const start_position = opt_start.get(0);
      auto const end_position = opt_end.get(start_position);
      return new Range(document, start_position, end_position);
    }
    auto const range = either_document_or_range.right;
    auto const document = range->document();
    if (opt_start.is_supplied) {
      return new Range(document, opt_start.value,
                       opt_end.get(opt_start.value));
    }
    return new Range(document, range->start(), range->end());
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("collapsed", &Range::collapsed)
        .SetProperty("document", &Range::document)
        .SetProperty("end", &Range::end, &Range::set_end)
        .SetProperty("start", &Range::start, &Range::set_start)
        .SetProperty("text", &Range::text, &Range::set_text)
        .SetMethod("collapseTo", &Range::CollapseTo)
        .SetMethod("copy", &Range::CopyToClipboard)
        .SetMethod("insertBefore", &Range::InsertBefore)
        .SetMethod("match_", &Range::Match)
        .SetMethod("paste", &Range::PasteFromClipboard)
        .SetMethod("style", &Range::SetStyle);
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
    : Range(document, document->buffer()->CreateRange(start, end)) {
}

Range::Range(Document* document, text::Range* range)
    : document_(document),
      range_(range) {
  DCHECK_EQ(document->buffer(), range->GetBuffer());
  document_->DidCreateRange(this);
}

Range::~Range() {
  document_->DidDestroyRange(this);
}

bool Range::collapsed() const {
  return range_->GetStart() == range_->GetEnd();
}

int Range::end() const {
  return range_->GetEnd();
}

int Range::start() const {
  return range_->GetStart();
}

base::string16 Range::text() const {
  return std::move(range_->GetText());
}

void Range::set_end(int position) {
  if (!document_->IsValidPosition(position))
    return;
  range_->SetEnd(position);
}

void Range::set_start(int position) {
  if (!document_->IsValidPosition(position))
    return;
  range_->SetStart(position);
}

void Range::set_text(const base::string16& text) {
  if (!document_->CheckCanChange())
    return;
  range_->SetText(text);
}

Range* Range::CollapseTo(Posn position) {
  if (!document_->IsValidPosition(position))
    return this;
  range_->SetRange(position, position);
  return this;
}

void Range::CopyToClipboard() const {
  range_->Copy();
}

Range* Range::InsertBefore(const base::string16& text) {
  if (!document_->CheckCanChange())
    return this;
  document_->buffer()->InsertBefore(start(), text);
  return this;
}

v8::Handle<v8::Value> Range::Match(RegExp* regexp) {
  return regexp->ExecuteOnDocument(document(), start(), end());
}

void Range::PasteFromClipboard() {
  if (!document_->CheckCanChange())
    return;
  range_->Paste();
}

}  // namespace dom
