// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/range.h"

#include "evita/dom/buffer.h"
#include "evita/dom/converter.h"
#include "evita/dom/document.h"
#include "evita/text/range.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
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

  private: static Range* NewRange(Document* document,
                                  v8_glue::Optional<int> start,
                                  v8_glue::Optional<int> end) {
    auto const start_position = start.get(0);
    auto const end_position = end.get(start_position);
    return new Range(document, start_position, end_position);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("document", &Range::document)
        .SetProperty("end", &Range::end, &Range::set_end)
        .SetProperty("start", &Range::start, &Range::set_start)
        .SetProperty("text", &Range::text, &Range::set_text)
        .SetMethod("collapseTo", &Range::CollapseTo)
        .SetMethod("copy", &Range::CopyToClipboard)
        .SetMethod("insertBefore", &Range::InsertBefore)
        .SetMethod("paste", &Range::PasteFromClipboard);
  }
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
  document_->buffer()->InsertBefore(start(), text);
  return this;
}

void Range::PasteFromClipboard() {
  range_->Paste();
}

}  // namespace dom
