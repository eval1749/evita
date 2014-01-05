// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/document.h"

#include <unordered_map>

#include "common/memory/singleton.h"
#include "evita/editor/application.h"
#include "evita/dom/buffer.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/wrapper_info.h"

namespace std {
template<> struct hash<Buffer*> {
  size_t operator()(const Buffer*& key) {
    return static_cast<size_t>(reinterpret_cast<uintptr_t>(key));
  }
};
}  // namespace std

namespace dom {

namespace {
v8::Handle<v8::Value> FindDocument(const string16& name) {
  auto const isolate = v8::Isolate::GetCurrent();
  auto const present = Application::instance()->FindBuffer(name);
  if (!present)
    return v8::Null(isolate);
  auto const document = Document::GetOrCreateDocument(present);
  return document->GetWrapper(isolate);
}

//////////////////////////////////////////////////////////////////////
//
// DocumentWrapperInfo
//
class DocumentWrapperInfo : public v8_glue::WrapperInfo {
  public: DocumentWrapperInfo() : v8_glue::WrapperInfo("Document") {
  }
  public: ~DocumentWrapperInfo() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto templ = v8_glue::CreateConstructorTemplate(isolate,
        &DocumentWrapperInfo::NewDocument);
    return v8_glue::FunctionTemplateBuilder(isolate, templ)
        .SetMethod("find", &FindDocument)
        .Build();
  }

  private: static Document* NewDocument(const base::string16& name) {
    return new Document(name);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("length", &Document::length)
        .SetProperty("name", &Document::name);
  }
};

class BufferToDocumentMapper
    : public common::Singleton<BufferToDocumentMapper> {
  private: typedef std::unordered_map<uintptr_t, Document*>
      BufferToDocumentMap;
  private: BufferToDocumentMap map_;

  private: static uintptr_t key(Buffer* buffer) {
    return reinterpret_cast<uintptr_t>(buffer);
  }

  public: Document* Find(Buffer* buffer) const {
    auto it = map_.find(key(buffer));
    return it == map_.end() ? nullptr : it->second;
  }

  public: void Register(Document* document) {
    map_[key(document->buffer())] = document;
  }

  public: void Unregister(Document* document) {
    auto it = map_.find(key(document->buffer()));
    if (it != map_.end())
      map_.erase(it);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Document
//
Document::Document(const base::string16 name)
    : Document(Application::instance()->NewBuffer(name)) {
}

Document::Document(Buffer* buffer)
    : buffer_(buffer) {
  BufferToDocumentMapper::instance()->Register(this);
}

Document::~Document() {
  BufferToDocumentMapper::instance()->Unregister(this);
}

int Document::length() const {
  return buffer_->GetEnd();
}

const base::string16& Document::name() const {
  return buffer_->name();
}

v8_glue::WrapperInfo* Document::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(DocumentWrapperInfo, wrapper_info, ());
  return &wrapper_info;
}

void Document::DidCreateRange(Range* range) {
  ranges_.insert(range);
}

void Document::DidDestroyRange(Range* range) {
  ranges_.erase(range);
}

Document* Document::GetOrCreateDocument(Buffer* buffer) {
  auto present = BufferToDocumentMapper::instance()->Find(buffer);
  return present ? present : new Document(buffer);
}

bool Document::IsValidPosition(Posn position) const {
  if (position >= 0 && position <= buffer_->GetEnd())
    return true;
  ScriptController::instance()->ThrowError("Invalid position.");
  return false;
}

}  // namespace dom
