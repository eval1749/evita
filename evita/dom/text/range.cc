// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/range.h"

#include "base/strings/stringprintf.h"
#include "evita/css/style_selector.h"
#include "evita/dom/converter.h"
#include "evita/dom/script_host.h"
#include "evita/dom/text/document.h"
#include "evita/text/buffer.h"
#include "evita/text/marker_set.h"
#include "evita/text/offset.h"
#include "evita/text/range.h"
#include "evita/text/spelling.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Range
//
Range::Range(Document* document, text::Offset start, text::Offset end)
    : Range(document, new text::Range(document->buffer(), start, end)) {}

Range::Range(Document* document, text::Range* range)
    : document_(document), range_(range) {}

Range::~Range() {}

bool Range::collapsed() const {
  return range_->start() == range_->end();
}

text::Offset Range::end() const {
  return range_->end();
}

text::Offset Range::start() const {
  return range_->start();
}

base::string16 Range::text() const {
  return std::move(range_->text());
}

void Range::set_end(int offsetLike) {
  const auto offset = document_->ValidateOffset(offsetLike);
  if (!offset.IsValid())
    return;
  range_->set_end(offset);
}

void Range::set_start(int offsetLike) {
  const auto offset = document_->ValidateOffset(offsetLike);
  if (!offset.IsValid())
    return;
  range_->set_start(offset);
}

void Range::set_text(const base::string16& text) {
  if (!document_->CheckCanChange())
    return;
  range_->set_text(text);
}

Range* Range::CollapseTo(int offsetLike) {
  const auto offset = document_->ValidateOffset(offsetLike);
  if (!offset.IsValid())
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
                       int offsetLike) {
  return NewRange(document_or_range, offsetLike, offsetLike);
}

Range* Range::NewRange(v8_glue::Either<Document*, Range*> document_or_range,
                       int startLike,
                       int endLike) {
  auto const document = document_or_range.is_left
                            ? document_or_range.left
                            : document_or_range.right->document();
  const auto start = document->ValidateOffset(startLike);
  if (!start.IsValid())
    return nullptr;
  const auto end = document->ValidateOffset(endLike);
  if (!end.IsValid())
    return nullptr;
  if (!document->IsValidRange(start, end))
    return nullptr;
  return new Range(document, start, end);
}

void Range::SetSpelling(int spelling_code) const {
  if (collapsed()) {
    ScriptHost::instance()->ThrowError(
        "Can't set spelling for collapsed range.");
    return;
  }
  document_->SetSpelling(range_->start(), range_->end(), spelling_code);
}

void Range::SetSyntax(const base::string16& syntax) const {
  if (collapsed()) {
    ScriptHost::instance()->ThrowError("Can't set syntax for collapsed range.");
    return;
  }
  document_->SetSyntax(range_->start(), range_->end(), syntax);
}

}  // namespace dom
