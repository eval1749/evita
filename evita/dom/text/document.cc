// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/document.h"

#include <utility>
#include <vector>

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "evita/css/style_selector.h"
#include "evita/dom/text/document_set.h"
#include "evita/dom/text/regular_expression.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"
#include "evita/metrics/time_scope.h"
#include "evita/text/buffer.h"
#include "evita/text/marker.h"
#include "evita/text/marker_set.h"
#include "evita/text/spelling.h"
#include "evita/v8_glue/runner.h"
#include "v8_strings.h"  // NOLINT(build/include)

namespace dom {

std::vector<Document*> Document::list() {
  return DocumentSet::instance()->list();
}

void Document::AddObserver(v8::Handle<v8::Function> function) {
  DocumentSet::instance()->AddObserver(function);
}

Document* Document::Find(const base::string16& name) {
  return DocumentSet::instance()->Find(name);
}

void Document::RemoveDocument(Document* document) {
  DocumentSet::instance()->Unregister(document);
}

void Document::RemoveObserver(v8::Handle<v8::Function> function) {
  DocumentSet::instance()->RemoveObserver(function);
}

//////////////////////////////////////////////////////////////////////
//
// Document
//
Document::Document(const base::string16& name)
    : buffer_(new text::Buffer()),
      name_(DocumentSet::instance()->MakeUniqueName(name)) {}

Document::~Document() {
  DocumentSet::instance()->Unregister(this);
}

base::char16 Document::charCodeAt(text::Posn position) const {
  if (position >= 0 && position < length())
    return buffer_->GetCharAt(position);
  ScriptHost::instance()->ThrowRangeError(
      base::StringPrintf("Bad index %d, valid index is [%d, %d]", position, 0,
                         buffer_->GetEnd() - 1));
  return 0;
}

text::Posn Document::length() const {
  return buffer_->GetEnd();
}

bool Document::modified() const {
  return buffer_->IsModified();
}

void Document::set_modified(bool new_modified) {
  buffer_->SetModified(new_modified);
}

const base::string16& Document::name() const {
  return name_;
}

void Document::set_name(const base::string16& new_name) {
  DCHECK_NE(name_, new_name);
  name_ = new_name;
}

bool Document::read_only() const {
  return buffer_->IsReadOnly();
}

void Document::set_read_only(bool read_only) const {
  buffer_->SetReadOnly(read_only);
}

const base::string16& Document::spelling_at(text::Posn offset) const {
  if (!IsValidPosition(offset))
    return common::AtomicString::Empty();
  auto const marker = buffer_->spelling_markers()->GetMarkerAt(offset);
  return marker ? marker->type() : common::AtomicString::Empty();
}

const base::string16& Document::syntax_at(text::Posn offset) const {
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

text::LineAndColumn Document::GetLineAndColumn(text::Posn offset) const {
  if (!IsValidPosition(offset))
    return buffer_->GetLineAndColumn(0);
  METRICS_TIME_SCOPE();
  return buffer_->GetLineAndColumn(offset);
}

bool Document::IsValidPosition(text::Posn position) const {
  if (position >= 0 && position <= buffer_->GetEnd())
    return true;
  ScriptHost::instance()->ThrowRangeError(
      base::StringPrintf("Invalid position %d, valid range is [%d, %d]",
                         position, 0, buffer_->GetEnd()));
  return false;
}

v8::Handle<v8::Value> Document::Match(RegularExpression* regexp,
                                      int start,
                                      int end) {
  return regexp->ExecuteOnDocument(this, start, end);
}

Document* Document::NewDocument(const base::string16& name) {
  auto const document = new Document(name);
  DocumentSet::instance()->Register(document);
  return document;
}

Posn Document::Redo(Posn position) {
  return buffer_->Redo(position);
}

void Document::RenameTo(const base::string16& new_name) {
  DocumentSet::instance()->RenameDocument(this, new_name);
}

base::string16 Document::Slice(int start, int end) {
  return buffer_->GetText(start, end);
}

base::string16 Document::Slice(int start) {
  return Slice(start, buffer_->GetEnd());
}

void Document::StartUndoGroup(const base::string16& name) {
  buffer_->StartUndoGroup(name);
}

Posn Document::Undo(Posn position) {
  return buffer_->Undo(position);
}

}  // namespace dom
