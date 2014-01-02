// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/range.h"

#include "common/memory/singleton.h"
#include "evita/editor/application.h"
#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/text/range.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/per_isolate_data.h"
#include "evita/v8_glue/wrapper_info.h"
#include "gin/arguments.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// RangeWrapperInfo
//
class RangeWrapperInfo : public v8_glue::WrapperInfo {
  public: RangeWrapperInfo() : v8_glue::WrapperInfo("Range") {
  }
  public: ~RangeWrapperInfo() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &RangeWrapperInfo::NewRange);
  }

  private: static Range* NewRange(Document* document,
                                  gin::Optional<int> start,
                                  gin::Optional<int> end) {
    auto const start_position = start.get(0);
    auto const end_position = end.get(start_position);
    return new Range(document, start_position, end_position);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("document", &Range::document)
        .SetProperty("end", &Range::end)
        .SetProperty("start", &Range::start);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Range
//
Range::Range(Document* document, text::Posn start, text::Posn end)
    : document_(document),
      range_(document->buffer()->CreateRange(start, end)) {
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

v8_glue::WrapperInfo* Range::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(RangeWrapperInfo, wrapper_info, ());
  return &wrapper_info;
}

}  // namespace dom
