// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/text_document.h"

#include <utility>
#include <vector>

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "evita/css/style_selector.h"
#include "evita/dom/text/regular_expression.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"
#include "evita/metrics/time_scope.h"
#include "evita/text/buffer.h"
#include "evita/text/marker.h"
#include "evita/text/marker_set.h"
#include "evita/text/offset.h"
#include "evita/text/spelling.h"
#include "evita/text/static_range.h"
#include "evita/v8_glue/runner.h"
#include "v8_strings.h"  // NOLINT(build/include)

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
  exception_state->ThrowRangeError(
      base::StringPrintf("Bad index %d, valid index is [%d, %d]", position, 0,
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

const base::string16& TextDocument::spelling_at(
    text::Offset offset,
    ExceptionState* exception_state) const {
  if (!IsValidPosition(offset, exception_state))
    return common::AtomicString::Empty();
  auto const marker = buffer_->spelling_markers()->GetMarkerAt(offset);
  return marker ? marker->type() : common::AtomicString::Empty();
}

const base::string16& TextDocument::syntax_at(
    text::Offset offset,
    ExceptionState* exception_state) const {
  if (!IsValidPosition(offset, exception_state))
    return common::AtomicString::Empty();
  auto const marker = buffer_->syntax_markers()->GetMarkerAt(offset);
  return marker ? marker->type() : css::StyleSelector::normal();
}

bool TextDocument::CheckCanChange(ExceptionState* exception_state) const {
  if (buffer_->IsReadOnly()) {
    auto const runner = ScriptHost::instance()->runner();
    auto const isolate = runner->isolate();
    v8_glue::Runner::Scope runner_scope(runner);
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

bool TextDocument::IsValidPosition(text::Offset offset,
                                   ExceptionState* exception_state) const {
  if (offset.IsValid() && offset <= buffer_->GetEnd())
    return true;
  exception_state->ThrowRangeError(
      base::StringPrintf("Invalid offset %d, valid range is [%d, %d]",
                         offset.value(), 0, buffer_->GetEnd()));
  return false;
}

bool TextDocument::IsValidRange(text::Offset start,
                                text::Offset end,
                                ExceptionState* exception_state) const {
  if (start <= end && end <= buffer_->GetEnd())
    return true;
  exception_state->ThrowRangeError(
      base::StringPrintf("Invalid range %d, valid range is [%d, %d]",
                         end.value(), start.value(), buffer_->GetEnd()));
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

void TextDocument::SetSpelling(text::Offset start,
                               text::Offset end,
                               int spelling_code,
                               ExceptionState* exception_state) {
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
  if (!IsValidRange(start, end, exception_state))
    return;
  buffer()->spelling_markers()->InsertMarker(
      text::StaticRange(*buffer(), start, end),
      Local::MapToSpelling(spelling_code));
}

void TextDocument::SetSyntax(text::Offset start,
                             text::Offset end,
                             const base::string16& syntax,
                             ExceptionState* exception_state) {
  if (!IsValidRange(start, end, exception_state))
    return;
  buffer()->syntax_markers()->InsertMarker(
      text::StaticRange(*buffer(), start, end), common::AtomicString(syntax));
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
                         offsetLike, 0, buffer_->GetEnd()));
  return text::Offset::Invalid();
}

}  // namespace dom
