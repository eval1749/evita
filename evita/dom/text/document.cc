// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/document.h"

#include <utility>

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/strings/stringprintf.h"
#include "evita/editor/application.h"
#include "evita/dom/text/buffer.h"
#include "evita/dom/converter.h"
#include "evita/dom/text/document_set.h"
#include "evita/dom/text/modes/mode.h"
#include "evita/dom/public/api_callback.h"
#include "evita/dom/text/regexp.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/text/marker.h"
#include "evita/text/marker_set.h"
#include "evita/text/modes/mode.h"
#include "evita/text/spelling.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/nullable.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/script_callback.h"
#include "evita/v8_glue/wrapper_info.h"
#include "v8_strings.h"

namespace dom {

namespace {
v8::Handle<v8::Object> NewMap(v8::Isolate* isolate) {
  return isolate->GetCurrentContext()->Global()->
      Get(v8Strings::Map.Get(isolate))->ToObject()->
          CallAsConstructor(0, nullptr)->ToObject();
}

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

  private: static Document* GetOrNew(const base::string16& name,
                                     v8_glue::Optional<Mode*> opt_mode);
  private: static Document* NewDocument(const base::string16& name,
                                        v8_glue::Optional<Mode*> opt_mode);
  private: static void RemoveDocument(Document* document);
  private: static void RemoveObserver(v8::Handle<v8::Function> function);

  // v8_glue::WrapperInfo
  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override;

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override;

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

Document* DocumentClass::GetOrNew(const base::string16& name,
                                  v8_glue::Optional<Mode*> opt_mode) {
  if (auto const document = DocumentSet::instance()->Find(name))
    return document;
  return NewDocument(name, opt_mode);
}

Document* DocumentClass::NewDocument(const base::string16& name,
                                     v8_glue::Optional<Mode*> opt_mode) {
  if (opt_mode.is_supplied)
    return Document::New(name, opt_mode.value);

  // Get mode by |Mode.chooseModeByFileName()|.
  auto const runner = ScriptController::instance()->runner();
  auto const isolate = runner->isolate();
  auto const js_mode_class = runner->global()->Get(
      v8Strings::Mode.Get(isolate));
  auto const js_choose = js_mode_class->ToObject()->Get(
      v8Strings::chooseModeByFileName.Get(isolate));
  auto const js_name = gin::StringToV8(isolate, name);
  auto const js_mode = runner->Call(js_choose, js_mode_class, js_name);
  Mode* mode;
  if (!gin::ConvertFromV8(isolate, js_mode, &mode)) {
    ScriptController::instance()->ThrowException(v8::Exception::TypeError(
        v8Strings::Mode.Get(isolate)));
    return nullptr;
  }
  return Document::New(name, mode);
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
      &DocumentClass::NewDocument);
  return v8_glue::FunctionTemplateBuilder(isolate, templ)
      .SetProperty("list", &DocumentClass::list)
      .SetMethod("addObserver", &DocumentClass::AddObserver)
      .SetMethod("find", &DocumentClass::Find)
      .SetMethod("getOrNew", &DocumentClass::GetOrNew)
      .SetMethod("remove", &DocumentClass::RemoveDocument)
      .SetMethod("removeObserver", &DocumentClass::RemoveObserver)
      .Build();
}

void DocumentClass::SetupInstanceTemplate(ObjectTemplateBuilder& builder) {
  builder
      .SetProperty("filename", &Document::filename, &Document::set_filename)
      .SetProperty("lastWriteTime", &Document::last_write_time,
                   &Document::set_last_write_time)
      .SetProperty("length", &Document::length)
      .SetProperty("mode", &Document::mode, &Document::set_mode)
      .SetProperty("modified", &Document::modified)
      .SetProperty("name", &Document::name)
      .SetProperty("properties", &Document::properties)
      .SetProperty("readonly", &Document::read_only,
                   &Document::set_read_only)
      .SetMethod("charCodeAt_", &Document::charCodeAt)
      .SetMethod("doColor_", &Document::DoColor)
      .SetMethod("endUndoGroup_", &Document::EndUndoGroup)
      .SetMethod("load_", &Document::Load)
      .SetMethod("match_", &Document::Match)
      .SetMethod("redo", &Document::Redo)
      .SetMethod("renameTo", &Document::RenameTo)
      .SetMethod("save_", &Document::Save)
      .SetMethod("slice", &Document::Slice)
      .SetMethod("startUndoGroup_", &Document::StartUndoGroup)
      .SetMethod("spellingAt", &Document::spelling_at)
      .SetMethod("styleAt", &Document::style_at)
      .SetMethod("undo", &Document::Undo);
}


//////////////////////////////////////////////////////////////////////
//
// LoadFileCallback
//
class LoadFileCallback : public base::RefCounted<LoadFileCallback> {
  private: gc::Member<Document> document_;
  private: v8_glue::ScopedPersistent<v8::Function> function_;
  private: base::WeakPtr<v8_glue::Runner> runner_;

  public: LoadFileCallback(v8_glue::Runner* runner,
                           Document* document,
                           v8::Handle<v8::Function> function)
    : document_(document), function_(runner->isolate(), function),
      runner_(runner->GetWeakPtr()) {
  }

  public: void Run(const domapi::LoadFileCallbackData& data) {
    auto const buffer = document_->buffer();
    if (!data.error_code) {
      buffer->SetCodePage(static_cast<uint32_t>(data.code_page));
      buffer->SetNewline(data.newline_mode);
      buffer->SetFile(buffer->GetFileName(), data.last_write_time);
      buffer->SetReadOnly(data.readonly);
      // Noet: There is no good reason to undo of file loading. Although, users
      // can undo failure loading.
      buffer->ClearUndo();
    }
    buffer->FinishIo(static_cast<uint32_t>(data.error_code));
    if (!runner_)
      return;
    v8_glue::Runner::Scope runner_scope(runner_.get());
    auto const isolate = runner_->isolate();
    auto const function = function_.NewLocal(isolate);
    runner_->Call(function, document_->GetWrapper(isolate),
                  v8::Integer::New(isolate, data.error_code));
  }

  DISALLOW_COPY_AND_ASSIGN(LoadFileCallback);
};

//////////////////////////////////////////////////////////////////////
//
// SaveFileCallback
//
class SaveFileCallback : public base::RefCounted<SaveFileCallback> {
  private: gc::Member<Document> document_;
  private: v8_glue::ScopedPersistent<v8::Function> function_;
  private: base::WeakPtr<v8_glue::Runner> runner_;

  public: SaveFileCallback(v8_glue::Runner* runner,
                           Document* document,
                           v8::Handle<v8::Function> function)
    : document_(document), function_(runner->isolate(), function),
      runner_(runner->GetWeakPtr()) {
  }

  public: void Run(const domapi::SaveFileCallbackData& data) {
    auto const buffer = document_->buffer();
    if (!data.error_code)
      buffer->SetFile(buffer->GetFileName(), data.last_write_time);
    buffer->FinishIo(static_cast<uint32_t>(data.error_code));
    if (!runner_)
      return;
    v8_glue::Runner::Scope runner_scope(runner_.get());
    auto const isolate = runner_->isolate();
    auto const function = function_.NewLocal(isolate);
    runner_->Call(function, document_->GetWrapper(isolate),
                  v8::Integer::New(isolate, data.error_code));
  }

  DISALLOW_COPY_AND_ASSIGN(SaveFileCallback);
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Document
//
DEFINE_SCRIPTABLE_OBJECT(Document, DocumentClass)

Document::Document(const base::string16& name, Mode* mode)
    : buffer_(new Buffer(DocumentSet::instance()->MakeUniqueName(name),
                         mode->text_mode())),
      mode_(mode),
      properties_(v8::Isolate::GetCurrent(),
                  NewMap(v8::Isolate::GetCurrent())) {
}

Document::~Document() {
  DocumentSet::instance()->Unregister(this);
}

base::char16 Document::charCodeAt(text::Posn position) const {
  if (position >= 0 && position < length())
    return buffer_->GetCharAt(position);
  auto const isolate = v8::Isolate::GetCurrent();
  auto const error = v8::Exception::RangeError(
    gin::StringToV8(isolate, base::StringPrintf(
      "Bad index %d, valid index is [%d, %d]",
      position, 0, buffer_->GetEnd() - 1)));
  ScriptController::instance()->ThrowException(error);
  return 0;
}

const base::string16& Document::filename() const {
  return buffer_->GetFileName();
}

void Document::set_filename(const base::string16& filename) {
  return buffer_->set_filename(filename);
}

base::Time Document::last_write_time() const {
  return buffer_->GetLastWriteTime();
}

void Document::set_last_write_time(base::Time last_write_time) {
  return buffer_->SetFile(buffer_->GetFileName(), last_write_time);
}

text::Posn Document::length() const {
  return buffer_->GetEnd();
}

void Document::set_mode(Mode* mode) {
  mode_ = mode;
  buffer_->SetMode(mode_->text_mode());
}

bool Document::modified() const {
  return buffer_->IsModified();
}

const base::string16& Document::name() const {
  return buffer_->name();
}

v8::Handle<v8::Object> Document::properties() const {
  return properties_.NewLocal(v8::Isolate::GetCurrent());
}

bool Document::read_only() const {
  return buffer_->IsReadOnly();
}

void Document::set_read_only(bool read_only) const {
  buffer_->SetReadOnly(read_only);
}

int Document::spelling_at(text::Posn offset) const {
  if (!IsValidPosition(offset))
    return 0;
  auto const marker = buffer_->spelling_markers()->GetMarkerAt(offset);
  return marker ? marker->type() : static_cast<int>(text::Spelling::None);
}

bool Document::CheckCanChange() const {
  if (buffer_->IsReadOnly()) {
    auto const runner = ScriptController::instance()->runner();
    auto const isolate = runner->isolate();
    v8_glue::Runner::Scope runner_scope(runner);
    auto const ctor = runner->global()->Get(
        v8Strings::DocumentReadOnly.Get(isolate));
    auto const error = runner->CallAsConstructor(ctor, GetWrapper(isolate));
    ScriptController::instance()->ThrowException(error);
    return false;
  }
  return true;
}

void Document::DoColor(int hint) {
  buffer_->GetMode()->DoColor(hint);
}

void Document::EndUndoGroup(const base::string16& name) {
  buffer_->EndUndoGroup(name);
}

Document* Document::Find(const base::string16& name) {
  return DocumentSet::instance()->Find(name);
}

bool Document::IsValidPosition(text::Posn position) const {
  if (position >= 0 && position <= buffer_->GetEnd())
    return true;
  auto const isolate = v8::Isolate::GetCurrent();
  auto const error = v8::Exception::RangeError(
    gin::StringToV8(isolate, base::StringPrintf(
      "Invalid position %d, valid range is [%d, %d]",
      position, 0, buffer_->GetEnd())));
  ScriptController::instance()->ThrowException(error);
  return false;
}

void Document::Load(const base::string16& filename,
                    v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const load_callback = make_scoped_refptr(
      new LoadFileCallback(runner, this, callback));
  buffer()->Load(filename, base::Bind(&LoadFileCallback::Run, load_callback));
}

v8::Handle<v8::Value> Document::Match(RegExp* regexp, int start, int end) {
  return regexp->ExecuteOnDocument(this, start, end);
}

Document* Document::New(const base::string16& name, Mode* mode) {
  auto const document = new Document(name, mode);
  DocumentSet::instance()->Register(document);
  return document;
}

Posn Document::Redo(Posn position) {
  return buffer_->Redo(position);
}

void Document::RenameTo(const base::string16& new_name) {
  DocumentSet::instance()->RenameDocument(this, new_name);
}

void Document::ResetForTesting() {
  DocumentSet::instance()->ResetForTesting();
}

void Document::Save(const base::string16& filename,
                    v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const save_callback = make_scoped_refptr(
      new SaveFileCallback(runner, this, callback));
  auto const code_page = buffer()->GetCodePage() ?
      static_cast<int>(buffer()->GetCodePage()) : 932;
  auto const newline_mode = buffer()->GetNewline() == NewlineMode_Detect ?
        NewlineMode_CrLf : buffer()->GetNewline();
  buffer()->Save(filename, code_page, newline_mode,
                 base::Bind(&SaveFileCallback::Run, save_callback));
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
