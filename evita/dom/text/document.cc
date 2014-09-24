// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/document.h"

#include <utility>

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/strings/stringprintf.h"
#include "evita/css/style_selector.h"
#include "evita/editor/application.h"
#include "evita/dom/converter.h"
#include "evita/dom/text/document_set.h"
#include "evita/dom/text/regexp.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"
#include "evita/metrics/time_scope.h"
#include "evita/text/buffer.h"
#include "evita/text/marker.h"
#include "evita/text/marker_set.h"
#include "evita/text/spelling.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"

#include "evita/v8_glue/nullable.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/wrapper_info.h"
#include "v8_strings.h"

namespace gin {
template<>
struct Converter<text::LineAndColumn> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      text::LineAndColumn& line_and_column) {
    auto const result = v8::Object::New(isolate);
    result->Set(gin::StringToV8(isolate, "column"),
                v8::Integer::New(isolate, line_and_column.column));
    result->Set(gin::StringToV8(isolate, "lineNumber"),
                v8::Integer::New(isolate, line_and_column.line_number));
    return result;
  }
};
}  // namespace gin

namespace dom {

namespace bindings {
//////////////////////////////////////////////////////////////////////
//
// DocumentClass
//
class DocumentClass
    : public v8_glue::DerivedWrapperInfo<Document, EventTarget> {
  public: DocumentClass(const char* name);
  public: virtual ~DocumentClass();

  private: static std::vector<Document*> list();

  private: static void AddObserver(v8::Handle<v8::Function> function);
  private: static v8_glue::Nullable<Document> Find(const base::string16& name);

  private: static Document* NewDocument(const base::string16& name);
  private: static void RemoveDocument(Document* document);
  private: static void RemoveObserver(v8::Handle<v8::Function> function);

  // v8_glue::WrapperInfo
  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override;

  private: virtual v8::Handle<v8::ObjectTemplate> SetupInstanceTemplate(
      v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ) override;

  DISALLOW_COPY_AND_ASSIGN(DocumentClass);
};

DocumentClass::DocumentClass(const char* name)
    : BaseClass(name) {
}

DocumentClass::~DocumentClass() {
}

std::vector<Document*> DocumentClass::list() {
  return DocumentSet::instance()->list();
}

void DocumentClass::AddObserver(v8::Handle<v8::Function> function) {
  DocumentSet::instance()->AddObserver(function);
}

v8_glue::Nullable<Document> DocumentClass::Find(const base::string16& name) {
  return DocumentSet::instance()->Find(name);
}

void DocumentClass::RemoveDocument(Document* document) {
  DocumentSet::instance()->Unregister(document);
}

void DocumentClass::RemoveObserver(v8::Handle<v8::Function> function) {
  DocumentSet::instance()->RemoveObserver(function);
}

// v8_glue::WrapperInfo
v8::Handle<v8::FunctionTemplate> DocumentClass::CreateConstructorTemplate(
    v8::Isolate* isolate) {
  auto templ = v8_glue::CreateConstructorTemplate(isolate,
      &Document::New);
  return v8_glue::FunctionTemplateBuilder(isolate, templ)
      .SetProperty("list", &DocumentClass::list)
      .SetMethod("addObserver", &DocumentClass::AddObserver)
      .SetMethod("find", &DocumentClass::Find)
      .SetMethod("remove", &DocumentClass::RemoveDocument)
      .SetMethod("removeObserver", &DocumentClass::RemoveObserver)
      .Build();
}

v8::Handle<v8::ObjectTemplate>
DocumentClass::SetupInstanceTemplate(v8::Isolate* isolate,
                                     v8::Handle<v8::ObjectTemplate> templ) {
  gin::ObjectTemplateBuilder builder(isolate, templ);
  builder
      .SetProperty("length", &Document::length)
      .SetProperty("modified", &Document::modified, &Document::set_modified)
      .SetProperty("name", &Document::name)
      .SetProperty("readonly", &Document::read_only,
                   &Document::set_read_only)
      .SetMethod("charCodeAt_", &Document::charCodeAt)
      .SetMethod("clearUndo", &Document::ClearUndo)
      .SetMethod("endUndoGroup_", &Document::EndUndoGroup)
      .SetMethod("getLineAndColumn_", &Document::GetLineAndColumn)
      .SetMethod("match_", &Document::Match)
      .SetMethod("redo", &Document::Redo)
      .SetMethod("renameTo", &Document::RenameTo)
      .SetMethod("slice", &Document::Slice)
      .SetMethod("startUndoGroup_", &Document::StartUndoGroup)
      .SetMethod("spellingAt", &Document::spelling_at)
      .SetMethod("styleAt", &Document::style_at)
      .SetMethod("syntaxAt", &Document::syntax_at)
      .SetMethod("undo", &Document::Undo);
  return builder.Build();
}

}  // namespace bindings

//////////////////////////////////////////////////////////////////////
//
// Document
//
using namespace bindings;
DEFINE_SCRIPTABLE_OBJECT(Document, DocumentClass)

Document::Document(const base::string16& name)
    : buffer_(new text::Buffer()),
      name_(DocumentSet::instance()->MakeUniqueName(name)) {
}

Document::~Document() {
  DocumentSet::instance()->Unregister(this);
}

base::char16 Document::charCodeAt(text::Posn position) const {
  if (position >= 0 && position < length())
    return buffer_->GetCharAt(position);
  ScriptHost::instance()->ThrowRangeError(base::StringPrintf(
      "Bad index %d, valid index is [%d, %d]",
      position, 0, buffer_->GetEnd() - 1));
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
    auto const ctor = runner->global()->Get(
        v8Strings::DocumentReadOnly.Get(isolate));
    auto const error = runner->CallAsConstructor(ctor, GetWrapper(isolate));
    ScriptHost::instance()->ThrowException(error);
    return false;
  }
  return true;
}

void Document::ClearUndo(){
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
  ScriptHost::instance()->ThrowRangeError(base::StringPrintf(
      "Invalid position %d, valid range is [%d, %d]",
      position, 0, buffer_->GetEnd()));
  return false;
}

v8::Handle<v8::Value> Document::Match(RegExp* regexp, int start, int end) {
  return regexp->ExecuteOnDocument(this, start, end);
}

Document* Document::New(const base::string16& name) {
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

base::string16 Document::Slice(int start, v8_glue::Optional<int> opt_end) {
  auto const end = opt_end.is_supplied ? opt_end.value : buffer_->GetEnd();
  return buffer_->GetText(start, end);
}

void Document::StartUndoGroup(const base::string16& name) {
  buffer_->StartUndoGroup(name);
}

Posn Document::Undo(Posn position) {
  return buffer_->Undo(position);
}

}  // namespace dom
