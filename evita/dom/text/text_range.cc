// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/text_range.h"

#include "base/strings/stringprintf.h"
#include "evita/css/style_selector.h"
#include "evita/dom/converter.h"
#include "evita/dom/script_host.h"
#include "evita/dom/text/text_document.h"
#include "evita/text/buffer.h"
#include "evita/text/marker_set.h"
#include "evita/text/offset.h"
#include "evita/text/range.h"
#include "evita/text/spelling.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextRange
//
TextRange::TextRange(TextDocument* document,
                     text::Offset start,
                     text::Offset end)
    : TextRange(document, new text::Range(document->buffer(), start, end)) {}

TextRange::TextRange(TextDocument* document, text::Range* range)
    : document_(document), range_(range) {}

TextRange::~TextRange() {}

bool TextRange::collapsed() const {
  return range_->start() == range_->end();
}

text::Offset TextRange::end() const {
  return range_->end();
}

int TextRange::end_value() const {
  return end().value();
}

text::Offset TextRange::start() const {
  return range_->start();
}

int TextRange::start_value() const {
  return start().value();
}

base::string16 TextRange::text() const {
  return std::move(range_->text());
}

void TextRange::set_end(int offsetLike) {
  const auto offset = document_->ValidateOffset(offsetLike);
  if (!offset.IsValid())
    return;
  range_->set_end(offset);
}

void TextRange::set_start(int offsetLike) {
  const auto offset = document_->ValidateOffset(offsetLike);
  if (!offset.IsValid())
    return;
  range_->set_start(offset);
}

void TextRange::set_text(const base::string16& text) {
  if (!document_->CheckCanChange())
    return;
  range_->set_text(text);
}

TextRange* TextRange::CollapseTo(int offsetLike) {
  const auto offset = document_->ValidateOffset(offsetLike);
  if (!offset.IsValid())
    return this;
  range_->SetRange(offset, offset);
  return this;
}

TextRange* TextRange::InsertBefore(const base::string16& text) {
  if (!document_->CheckCanChange())
    return this;
  document_->buffer()->InsertBefore(start(), text);
  return this;
}

TextRange* TextRange::NewTextRange(
    v8_glue::Either<TextDocument*, TextRange*> document_or_range) {
  if (document_or_range.is_left)
    return NewTextRange(document_or_range, 0, 0);
  auto const range = document_or_range.right;
  return new TextRange(range->document(), range->start(), range->end());
}

TextRange* TextRange::NewTextRange(
    v8_glue::Either<TextDocument*, TextRange*> document_or_range,
    int offsetLike) {
  return NewTextRange(document_or_range, offsetLike, offsetLike);
}

TextRange* TextRange::NewTextRange(
    v8_glue::Either<TextDocument*, TextRange*> document_or_range,
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
  return new TextRange(document, start, end);
}

void TextRange::SetSpelling(int spelling_code) const {
  if (collapsed()) {
    ScriptHost::instance()->ThrowError(
        "Can't set spelling for collapsed range.");
    return;
  }
  document_->SetSpelling(range_->start(), range_->end(), spelling_code);
}

void TextRange::SetSyntax(const base::string16& syntax) const {
  if (collapsed()) {
    ScriptHost::instance()->ThrowError("Can't set syntax for collapsed range.");
    return;
  }
  document_->SetSyntax(range_->start(), range_->end(), syntax);
}

}  // namespace dom
