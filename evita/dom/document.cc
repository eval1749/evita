// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/document.h"

#include <unordered_map>
#include <utility>
#include <vector>

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "common/memory/singleton.h"
#include "evita/editor/application.h"
#include "evita/dom/buffer.h"
#include "evita/dom/converter.h"
#include "evita/dom/script_command.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/ed_Mode.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/nullable.h"
#include "evita/v8_glue/function_template_builder.h"
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
// DocumentList
//
class DocumentList : public common::Singleton<DocumentList> {
  friend class common::Singleton<DocumentList>;

  private: std::unordered_map<base::string16, Document*> map_;

  private: DocumentList() = default;
  public: ~DocumentList() = default;

  private: std::vector<Document*> list() const {
    std::vector<Document*> list(map_.size());
    list.resize(0);
    for (const auto& pair : map_) {
      list.push_back(pair.second);
    }
    return std::move(list);
  }

  public: Document* Find(const base::string16 name) const {
    auto it = map_.find(name);
    return it == map_.end() ? nullptr : it->second;
  }
  public: base::string16 MakeUniqueName(const base::string16& name) {
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
  public: void Register(Document* document) {
    CHECK(!Find(document->name()));
    map_[document->name()] = document;
  }
  public: void ResetForTesting() {
    map_.clear();
  }
  public: static std::vector<Document*> StaticList() {
    return instance()->list();
  }
  public: static v8_glue::Nullable<Document> StaticFind(
      const base::string16& name) {
    return instance()->Find(name);
  }
  public: static void StaticRemove(Document* document) {
    instance()->Unregister(document);
  }
  public: void Unregister(Document* document) {
    auto it = map_.find(document->name());
    if (it == map_.end()) {
      // We called |Document.remove()| for |document|.
      return;
    }
    map_.erase(it);
  }

  DISALLOW_COPY_AND_ASSIGN(DocumentList);
};

//////////////////////////////////////////////////////////////////////
//
// DocumentWrapperInfo
//
class DocumentWrapperInfo : public v8_glue::WrapperInfo {
  public: DocumentWrapperInfo(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~DocumentWrapperInfo() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto templ = v8_glue::CreateConstructorTemplate(isolate,
        &DocumentWrapperInfo::NewDocument);
    return v8_glue::FunctionTemplateBuilder(isolate, templ)
        .SetMethod("find", &DocumentList::StaticFind)
        .SetMethod("getOrNew", &Document::GetOrNew)
        .SetProperty("list", &DocumentList::StaticList)
        .SetMethod("remove", &DocumentList::StaticRemove)
        .Build();
  }

  private: static Document* NewDocument(const base::string16& name) {
    return new Document(name);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("filename", &Document::filename, &Document::set_filename)
        .SetProperty("length", &Document::length)
        .SetProperty("modified", &Document::modified)
        .SetProperty("name", &Document::name)
        .SetProperty("properties", &Document::properties)
        .SetMethod("bindKey_", &Document::BindKey)
        .SetMethod("charCodeAt_", &Document::charCodeAt)
        .SetMethod("doColor_", &Document::DoColor)
        .SetMethod("endUndoGroup_", &Document::EndUndoGroup)
        .SetMethod("load_", &Document::Load)
        .SetMethod("redo", &Document::Redo)
        .SetMethod("reload_", &Document::Reload)
        .SetMethod("renameTo", &Document::RenameTo)
        .SetMethod("save", &Document::Save)
        .SetMethod("startUndoGroup_", &Document::StartUndoGroup)
        .SetMethod("styleAt", &Document::style_at)
        .SetMethod("undo", &Document::Undo);
  }

  DISALLOW_COPY_AND_ASSIGN(DocumentWrapperInfo);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Document
//
DEFINE_SCRIPTABLE_OBJECT(Document, DocumentWrapperInfo)

Document::Document(const base::string16& name)
    : buffer_(new Buffer(DocumentList::instance()->MakeUniqueName(name))),
      properties_(v8::Isolate::GetCurrent(),
                  NewMap(v8::Isolate::GetCurrent())) {
  DocumentList::instance()->Register(this);
}

Document::~Document() {
  DocumentList::instance()->Unregister(this);
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

text::Posn Document::length() const {
  return buffer_->GetEnd();
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

void Document::BindKey(int key_code, v8::Handle<v8::Object> command) {
  buffer_->BindKey(static_cast<uint32>(key_code), new ScriptCommand(command));
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
  return DocumentList::instance()->Find(name);
}

Document* Document::GetOrNew(const base::string16& name) {
  if (auto const document = Find(name))
    return document;
  return new Document(name);
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

void Document::Load(const base::string16& filename) {
  // TODO(yosi) We should protect this document againt gc.
  ScriptController::instance()->view_delegate()->LoadFile(this, filename);
}

Posn Document::Redo(Posn position) {
  return buffer_->Redo(position);
}

void Document::Reload() {
  ScriptController::instance()->view_delegate()->ReloadTextBuffer(
      buffer_.get());
}

void Document::RenameTo(const base::string16& new_name) {
  if (name() == new_name)
   return;
  auto& list = *DocumentList::instance();
  auto new_unique_name = list.MakeUniqueName(new_name);
  list.Unregister(this);
  buffer_->SetName(new_unique_name);
  list.Register(this);
}

void Document::ResetForTesting() {
  DocumentList::instance()->ResetForTesting();
}

void Document::Save(const base::string16& filename) {
  // TODO(yosi) We should protect this document againt gc.
  ScriptController::instance()->view_delegate()->SaveFile(this, filename);
}

void Document::StartUndoGroup(const base::string16& name) {
  buffer_->StartUndoGroup(name);
}

Posn Document::Undo(Posn position) {
  return buffer_->Undo(position);
}

}  // namespace dom
