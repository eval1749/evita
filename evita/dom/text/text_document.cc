// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/text_document.h"

#include <utility>
#include <vector>

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"
#include "evita/dom/text/regular_expression.h"
#include "evita/dom/v8_strings.h"
#include "evita/ginx/runner.h"
#include "evita/metrics/time_scope.h"
#include "evita/text/models/buffer.h"
#include "evita/text/models/marker.h"
#include "evita/text/models/marker_set.h"
#include "evita/text/models/offset.h"
#include "evita/text/models/spelling.h"
#include "evita/text/models/static_range.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextDocument
//
TextDocument::TextDocument() : buffer_(new text::Buffer()) {}

TextDocument::~TextDocument() {}

base::char16 TextDocument::charCodeAt(text::Offset position,
                                      ExceptionState* exception_state) const {
  if (position >= text::Offset(0) && position < buffer_->GetEnd())
    return buffer_->GetCharAt(position);
  exception_state->ThrowRangeError(base::StringPrintf(
      "Bad index %d, valid index is [%d, %d]", position.value(), 0,
      (buffer_->GetEnd() - text::OffsetDelta(1)).value()));
  return 0;
}

int TextDocument::length() const {
  return text::OffsetDelta(buffer_->GetEnd().value());
}

bool TextDocument::read_only() const {
  return buffer_->IsReadOnly();
}

int TextDocument::revision() const {
  return buffer_->revision();
}

void TextDocument::set_read_only(bool read_only) const {
  buffer_->SetReadOnly(read_only);
}

base::string16 TextDocument::SpellingAt(text::Offset offset,
                                        ExceptionState* exception_state) const {
  if (!IsValidPosition(offset, exception_state))
    return base::string16();
  if (auto marker = buffer_->spelling_markers()->GetMarkerAt(offset))
    return marker->type().as_string();
  return base::string16();
}

base::string16 TextDocument::SyntaxAt(text::Offset offset,
                                      ExceptionState* exception_state) const {
  if (!IsValidPosition(offset, exception_state))
    return base::string16();
  if (auto marker = buffer_->syntax_markers()->GetMarkerAt(offset))
    return marker->type().as_string();
  return L"normal";
}

bool TextDocument::CheckCanChange(ExceptionState* exception_state) const {
  if (buffer_->IsReadOnly()) {
    auto const runner = ginx::Runner::From(exception_state->context());
    auto const isolate = runner->isolate();
    ginx::Runner::Scope runner_scope(runner);
    auto const ctor =
        runner->global()->Get(v8Strings::TextDocumentReadOnly.Get(isolate));
    auto const error = runner->CallAsConstructor(ctor, GetWrapper(isolate));
    exception_state->ThrowException(error);
    return false;
  }
  return true;
}

void TextDocument::ClearUndo() {
  buffer_->ClearUndo();
}

void TextDocument::EndUndoGroup(const base::string16& name) {
  buffer_->EndUndoGroup(name);
}

text::LineAndColumn TextDocument::GetLineAndColumn(
    text::Offset offset,
    ExceptionState* exception_state) const {
  if (!IsValidPosition(offset, exception_state))
    return buffer_->GetLineAndColumn(text::Offset());
  METRICS_TIME_SCOPE();
  return buffer_->GetLineAndColumn(offset);
}

bool TextDocument::IsValidNonEmptyRange(text::Offset start,
                                        text::Offset end,
                                        ExceptionState* exception_state) const {
  if (end - start > 0 && end <= buffer_->GetEnd())
    return true;
  exception_state->ThrowRangeError(base::StringPrintf(
      "Should be non empty range [%d, %d]", start.value(), end.value()));
  return false;
}

bool TextDocument::IsValidPosition(text::Offset offset,
                                   ExceptionState* exception_state) const {
  if (offset.IsValid() && offset <= buffer_->GetEnd())
    return true;
  exception_state->ThrowRangeError(
      base::StringPrintf("Invalid offset %d, valid range is [%d, %d]",
                         offset.value(), 0, buffer_->GetEnd().value()));
  return false;
}

bool TextDocument::IsValidRange(text::Offset start,
                                text::Offset end,
                                ExceptionState* exception_state) const {
  if (start <= end && end <= buffer_->GetEnd())
    return true;
  exception_state->ThrowRangeError(base::StringPrintf(
      "Invalid range %d, valid range is [%d, %d]", end.value(), start.value(),
      buffer_->GetEnd().value()));
  return false;
}

v8::Local<v8::Value> TextDocument::Match(RegularExpression* regexp,
                                         text::Offset start,
                                         text::Offset end) {
  return regexp->ExecuteOnTextDocument(this, start, end);
}

TextDocument* TextDocument::NewTextDocument() {
  return new TextDocument();
}

text::Offset TextDocument::Redo(text::Offset position) {
  return buffer_->Redo(position);
}

void TextDocument::Replace(text::Offset start,
                           text::Offset end,
                           const base::string16& replacement,
                           ExceptionState* exception_state) {
  if (!IsValidRange(start, end, exception_state))
    return;
  buffer_->Replace(start, end, replacement);
}

void TextDocument::SetSpelling(text::Offset start,
                               text::Offset end,
                               const base::string16& spelling,
                               ExceptionState* exception_state) {
  if (!IsValidNonEmptyRange(start, end, exception_state))
    return;
  buffer()->spelling_markers()->InsertMarker(
      text::StaticRange(*buffer(), start, end), base::AtomicString(spelling));
}

void TextDocument::SetSyntax(text::Offset start,
                             text::Offset end,
                             const base::string16& syntax,
                             ExceptionState* exception_state) {
  if (!IsValidNonEmptyRange(start, end, exception_state))
    return;
  buffer()->syntax_markers()->InsertMarker(
      text::StaticRange(*buffer(), start, end), base::AtomicString(syntax));
}

base::string16 TextDocument::Slice(int startLike, int endLike) {
  auto const start =
      text::Offset(startLike >= 0 ? startLike : length() + startLike);
  auto const end = text::Offset(endLike >= 0 ? endLike : length() + endLike);
  if (!start.IsValid() || !end.IsValid() || start >= end)
    return base::string16();
  return buffer_->GetText(start, end);
}

base::string16 TextDocument::Slice(int startLike) {
  auto const start =
      text::Offset(startLike >= 0 ? startLike : length() + startLike);
  if (!start.IsValid() || start >= buffer_->GetEnd())
    return base::string16();
  return Slice(startLike, length());
}

void TextDocument::StartUndoGroup(const base::string16& name) {
  buffer_->StartUndoGroup(name);
}

text::Offset TextDocument::Undo(text::Offset position) {
  return buffer_->Undo(position);
}

text::Offset TextDocument::ValidateOffset(
    int offsetLike,
    ExceptionState* exception_state) const {
  if (offsetLike >= 0 && offsetLike <= length())
    return text::Offset(offsetLike);
  exception_state->ThrowRangeError(
      base::StringPrintf("Invalid offset %d, valid range is [%d, %d]",
                         offsetLike, 0, buffer_->GetEnd().value()));
  return text::Offset::Invalid();
}

}  // namespace dom
