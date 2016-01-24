// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/text_range.h"

#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/text/text_document.h"
#include "evita/text/buffer.h"
#include "evita/text/offset.h"
#include "evita/text/range.h"

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

int TextRange::end() const {
  return range_->end().value();
}

int TextRange::start() const {
  return range_->start().value();
}

base::string16 TextRange::text() const {
  return std::move(range_->text());
}

void TextRange::set_end(int offsetLike, ExceptionState* exception_state) {
  const auto offset = document_->ValidateOffset(offsetLike, exception_state);
  if (!offset.IsValid())
    return;
  range_->set_end(offset);
}

void TextRange::set_start(int offsetLike, ExceptionState* exception_state) {
  const auto offset = document_->ValidateOffset(offsetLike, exception_state);
  if (!offset.IsValid())
    return;
  range_->set_start(offset);
}

void TextRange::set_text(const base::string16& text,
                         ExceptionState* exception_state) {
  if (!document_->CheckCanChange(exception_state))
    return;
  range_->set_text(text);
}

TextRange* TextRange::CollapseTo(int offsetLike,
                                 ExceptionState* exception_state) {
  const auto offset = document_->ValidateOffset(offsetLike, exception_state);
  if (!offset.IsValid())
    return this;
  range_->SetRange(offset, offset);
  return this;
}

void TextRange::InsertBefore(const base::string16& text,
                             ExceptionState* exception_state) {
  if (!document_->CheckCanChange(exception_state))
    return;
  document_->buffer()->InsertBefore(range_->start(), text);
}

TextRange* TextRange::NewTextRange(
    v8_glue::Either<TextDocument*, TextRange*> document_or_range,
    ExceptionState* exception_state) {
  if (document_or_range.is_left)
    return NewTextRange(document_or_range, 0, 0);
  const auto range = document_or_range.right;
  return new TextRange(range->document(), range->range_->start(),
                       range->range_->end());
}

TextRange* TextRange::NewTextRange(
    v8_glue::Either<TextDocument*, TextRange*> document_or_range,
    int offsetLike,
    ExceptionState* exception_state) {
  return NewTextRange(document_or_range, offsetLike, offsetLike,
                      exception_state);
}

TextRange* TextRange::NewTextRange(
    v8_glue::Either<TextDocument*, TextRange*> document_or_range,
    int startLike,
    int endLike,
    ExceptionState* exception_state) {
  const auto document = document_or_range.is_left
                            ? document_or_range.left
                            : document_or_range.right->document();
  const auto start = document->ValidateOffset(startLike, exception_state);
  if (!start.IsValid())
    return nullptr;
  const auto end = document->ValidateOffset(endLike, exception_state);
  if (!end.IsValid())
    return nullptr;
  if (!document->IsValidRange(start, end, exception_state))
    return nullptr;
  return new TextRange(document, start, end);
}

void TextRange::SetSpelling(int spelling_code,
                            ExceptionState* exception_state) const {
  if (collapsed()) {
    exception_state->ThrowError("Can't set spelling for collapsed range.");
    return;
  }
  document_->SetSpelling(range_->start(), range_->end(), spelling_code,
                         exception_state);
}

void TextRange::SetSyntax(const base::string16& syntax,
                          ExceptionState* exception_state) const {
  if (collapsed()) {
    exception_state->ThrowError("Can't set syntax for collapsed range.");
    return;
  }
  document_->SetSyntax(range_->start(), range_->end(), syntax, exception_state);
}

}  // namespace dom
