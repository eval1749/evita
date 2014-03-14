// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/document.h"

#include <unordered_map>
#include <utility>
#include <vector>

#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "common/memory/singleton.h"
#include "evita/editor/application.h"
#include "evita/dom/buffer.h"
#include "evita/dom/converter.h"
#include "evita/dom/modes/mode.h"
#include "evita/dom/public/api_callback.h"
#include "evita/dom/regexp.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/spelling.h"
#include "evita/dom/view_delegate.h"
#include "evita/text/marker.h"
#include "evita/text/marker_set.h"
#include "evita/text/modes/mode.h"
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
typedef std::pair<base::string16, base::string16> StringPair;
StringPair SplitByDot(const base::string16& name) {
  const auto last_dot = name.rfind('.');
  if (!last_dot || last_dot == base::string16::npos)
    return StringPair(name, L"");
  return StringPair(name.substr(0, last_dot), name.substr(last_dot));
}

v8::Handle<v8::Object> NewMap(v8::Isolate* isolate) {
  return isolate->GetCurrentContext()->Global()->
      Get(v8Strings::Map.Get(isolate))->ToObject()->
          CallAsConstructor(0, nullptr)->ToObject();
}

//////////////////////////////////////////////////////////////////////
//
// DocumentSet
//
class DocumentSet : public common::Singleton<DocumentSet> {
  friend class common::Singleton<DocumentSet>;

  private: std::unordered_map<base::string16, Document*> map_;

  private: DocumentSet() = default;
  public: ~DocumentSet() = default;

  private: std::vector<Document*> list() const;

  public: Document* Find(const base::string16 name) const;
  public: base::string16 MakeUniqueName(const base::string16& name);
  public: void Register(Document* document);
  public: void ResetForTesting();
  public: static std::vector<Document*> StaticList();
  public: static v8_glue::Nullable<Document> StaticFind(
      const base::string16& name);
  public: static void StaticRemove(Document* document);
  public: void Unregister(Document* document);

  DISALLOW_COPY_AND_ASSIGN(DocumentSet);
};

std::vector<Document*> DocumentSet::list() const {
  std::vector<Document*> list(map_.size());
  list.resize(0);
  for (const auto& pair : map_) {
    list.push_back(pair.second);
  }
  return std::move(list);
}

Document* DocumentSet::Find(const base::string16 name) const {
  auto it = map_.find(name);
  return it == map_.end() ? nullptr : it->second;
}

base::string16 DocumentSet::MakeUniqueName(const base::string16& name) {
  if (!Find(name))
    return name;
  const auto pair = SplitByDot(name);
  auto candidate = name;
  for (auto n = 2; Find(candidate); ++ n) {
    candidate = pair.first + L" (" + base::IntToString16(n) + L")" +
      pair.second;
  }
  return candidate;
}

void DocumentSet::Register(Document* document) {
  CHECK(!Find(document->name()));
  map_[document->name()] = document;
}

void DocumentSet::ResetForTesting() {
  map_.clear();
}

std::vector<Document*> DocumentSet::StaticList() {
  return instance()->list();
}

v8_glue::Nullable<Document> DocumentSet::StaticFind(
    const base::string16& name) {
  return instance()->Find(name);
}

void DocumentSet::StaticRemove(Document* document) {
  instance()->Unregister(document);
}

void DocumentSet::Unregister(Document* document) {
  auto it = map_.find(document->name());
  if (it == map_.end()) {
    // We called |Document.remove()| for |document|.
    return;
  }
  map_.erase(it);
}

//////////////////////////////////////////////////////////////////////
//
// DocumentClass
//
class DocumentClass : public v8_glue::WrapperInfo {
  public: DocumentClass(const char* name);
  public: virtual ~DocumentClass();

  private: static Document* GetOrNew(const base::string16& name,
                                     v8_glue::Optional<Mode*> opt_mode);
  private: static Document* NewDocument(const base::string16& name,
                                        v8_glue::Optional<Mode*> opt_mode);

  // v8_glue::WrapperInfo
  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override;

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override;

  DISALLOW_COPY_AND_ASSIGN(DocumentClass);
};

DocumentClass::DocumentClass(const char* name)
    : v8_glue::WrapperInfo(name) {
}

DocumentClass::~DocumentClass() {
}

Document* DocumentClass::GetOrNew(const base::string16& name,
                                 v8_glue::Optional<Mode*> opt_mode) {
  if (auto const document = DocumentSet::StaticFind(name))
    return document;
  return NewDocument(name, opt_mode);
}

Document* DocumentClass::NewDocument(const base::string16& name,
                                     v8_glue::Optional<Mode*> opt_mode) {
  if (opt_mode.is_supplied)
    return new Document(name, opt_mode.value);

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
  return new Document(name, mode);
}

// v8_glue::WrapperInfo
v8::Handle<v8::FunctionTemplate> DocumentClass::CreateConstructorTemplate(
    v8::Isolate* isolate) {
  auto templ = v8_glue::CreateConstructorTemplate(isolate,
      &DocumentClass::NewDocument);
  return v8_glue::FunctionTemplateBuilder(isolate, templ)
      .SetProperty("list", &DocumentSet::StaticList)
      .SetMethod("find", &DocumentSet::StaticFind)
      .SetMethod("getOrNew", &GetOrNew)
      .SetMethod("remove", &DocumentSet::StaticRemove)
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
  DocumentSet::instance()->Register(this);
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
  return marker ? marker->type() : static_cast<int>(Spelling::None);
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

void Document::DidCreateRange(Range* range) {
  ranges_.insert(range);
}

void Document::DidDestroyRange(Range* range) {
  ranges_.erase(range);
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
  ScriptController::instance()->view_delegate()->LoadFile(this, filename,
      base::Bind(&LoadFileCallback::Run, load_callback));
}

v8::Handle<v8::Value> Document::Match(RegExp* regexp, int start, int end) {
  return regexp->ExecuteOnDocument(this, start, end);
}

Posn Document::Redo(Posn position) {
  return buffer_->Redo(position);
}

void Document::RenameTo(const base::string16& new_name) {
  if (name() == new_name)
   return;
  auto& list = *DocumentSet::instance();
  auto new_unique_name = list.MakeUniqueName(new_name);
  list.Unregister(this);
  buffer_->SetName(new_unique_name);
  list.Register(this);
}

void Document::ResetForTesting() {
  DocumentSet::instance()->ResetForTesting();
}

void Document::Save(const base::string16& filename,
                    v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const save_callback = make_scoped_refptr(
      new SaveFileCallback(runner, this, callback));
  ScriptController::instance()->view_delegate()->SaveFile(this, filename,
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
