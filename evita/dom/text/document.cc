// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/document.h"

#include <utility>
#include <vector>

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "evita/css/style_selector.h"
#include "evita/dom/text/regular_expression.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"
#include "evita/metrics/time_scope.h"
#include "evita/text/buffer.h"
#include "evita/text/marker.h"
#include "evita/text/marker_set.h"
#include "evita/text/offset.h"
#include "evita/text/spelling.h"
#include "evita/v8_glue/runner.h"
#include "v8_strings.h"  // NOLINT(build/include)

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Document
//
Document::Document() : buffer_(new text::Buffer()) {}

Document::~Document() {}

base::char16 Document::charCodeAt(text::Offset position) const {
  if (position >= 0 && position < length())
    return buffer_->GetCharAt(position);
  ScriptHost::instance()->ThrowRangeError(
      base::StringPrintf("Bad index %d, valid index is [%d, %d]", position, 0,
                         (buffer_->GetEnd() - text::OffsetDelta(1)).value()));
  return 0;
}

int Document::length() const {
  return text::OffsetDelta(buffer_->GetEnd().value());
}

bool Document::read_only() const {
  return buffer_->IsReadOnly();
}

int Document::revision() const {
  return buffer_->revision();
}

void Document::set_read_only(bool read_only) const {
  buffer_->SetReadOnly(read_only);
}

const base::string16& Document::spelling_at(text::Offset offset) const {
  if (!IsValidPosition(offset))
    return common::AtomicString::Empty();
  auto const marker = buffer_->spelling_markers()->GetMarkerAt(offset);
  return marker ? marker->type() : common::AtomicString::Empty();
}

const base::string16& Document::syntax_at(text::Offset offset) const {
  if (!IsValidPosition(offset))
    return common::AtomicString::Empty();
  auto const marker = buffer_->syntax_markers()->GetMarkerAt(offset);
  return marker ? marker->type() : css::StyleSelector::normal();
}

bool Document::CheckCanChange() const {
  if (buffer_->IsReadOnly()) {
    auto const runner = ScriptHost::instance()->runner();
    auto const isolate = runner->isolate();
    v8_glue::Runner::Scope runner_scope(runner);
    auto const ctor =
        runner->global()->Get(v8Strings::DocumentReadOnly.Get(isolate));
    auto const error = runner->CallAsConstructor(ctor, GetWrapper(isolate));
    ScriptHost::instance()->ThrowException(error);
    return false;
  }
  return true;
}

void Document::ClearUndo() {
  buffer_->ClearUndo();
}

void Document::EndUndoGroup(const base::string16& name) {
  buffer_->EndUndoGroup(name);
}

text::LineAndColumn Document::GetLineAndColumn(text::Offset offset) const {
  if (!IsValidPosition(offset))
    return buffer_->GetLineAndColumn(text::Offset());
  METRICS_TIME_SCOPE();
  return buffer_->GetLineAndColumn(offset);
}

bool Document::IsValidPosition(text::Offset offset) const {
  if (offset.IsValid() && offset <= buffer_->GetEnd())
    return true;
  ScriptHost::instance()->ThrowRangeError(
      base::StringPrintf("Invalid offset %d, valid range is [%d, %d]",
                         offset.value(), 0, buffer_->GetEnd()));
  return false;
}

bool Document::IsValidRange(text::Offset start, text::Offset end) const {
  if (start <= end)
    return true;
  ScriptHost::instance()->ThrowRangeError(
      base::StringPrintf("Invalid range %d, valid range is [%d, %d]",
                         end.value(), start.value(), buffer_->GetEnd()));
  return false;
}

v8::Handle<v8::Value> Document::Match(RegularExpression* regexp,
                                      text::Offset start,
                                      text::Offset end) {
  return regexp->ExecuteOnDocument(this, start, end);
}

Document* Document::NewDocument() {
  return new Document();
}

text::Offset Document::Redo(text::Offset position) {
  return buffer_->Redo(position);
}

void Document::SetSpelling(text::Offset start,
                           text::Offset end,
                           int spelling_code) {
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
  if (!IsValidPosition(start) || !IsValidPosition(end) || start >= end)
    return;
  buffer()->spelling_markers()->InsertMarker(
      start, end, Local::MapToSpelling(spelling_code));
}

void Document::SetSyntax(text::Offset start,
                         text::Offset end,
                         const base::string16& syntax) {
  if (!IsValidPosition(start) || !IsValidPosition(end) || start >= end)
    return;
  buffer()->syntax_markers()->InsertMarker(start, end,
                                           common::AtomicString(syntax));
}

base::string16 Document::Slice(int startLike, int endLike) {
  auto const start =
      text::Offset(startLike >= 0 ? startLike : length() + startLike);
  auto const end = text::Offset(endLike >= 0 ? endLike : length() + endLike);
  if (!start.IsValid() || !end.IsValid() || start >= end)
    return base::string16();
  return buffer_->GetText(start, end);
}

base::string16 Document::Slice(int startLike) {
  auto const start =
      text::Offset(startLike >= 0 ? startLike : length() + startLike);
  if (!start.IsValid() || start >= buffer_->GetEnd())
    return base::string16();
  return Slice(startLike, length());
}

void Document::StartUndoGroup(const base::string16& name) {
  buffer_->StartUndoGroup(name);
}

text::Offset Document::Undo(text::Offset position) {
  return buffer_->Undo(position);
}

text::Offset Document::ValidateOffset(int offsetLike) const {
  if (offsetLike >= 0 && offsetLike <= buffer_->GetEnd())
    return text::Offset(offsetLike);
  ScriptHost::instance()->ThrowRangeError(
      base::StringPrintf("Invalid offset %d, valid range is [%d, %d]",
                         offsetLike, 0, buffer_->GetEnd()));
  return text::Offset::Invalid();
}

}  // namespace dom
