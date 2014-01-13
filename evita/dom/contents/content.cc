// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/contents/content.h"

#include <unordered_map>

#include "base/logging.h"
#include "common/memory/singleton.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/nullable.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace contents {

namespace {
//////////////////////////////////////////////////////////////////////
//
// ContentList
//
class ContentList : public common::Singleton<ContentList> {
  friend class common::Singleton<ContentList>;

  private: std::unordered_map<base::string16, Content*> map_;

  private: ContentList() = default;
  public: ~ContentList() = default;

  private: std::vector<Content*> list() const {
    std::vector<Content*> list(map_.size());
    list.resize(0);
    for (const auto& pair : map_) {
      list.push_back(pair.second);
    }
    return std::move(list);
  }

  public: Content* Find(const base::string16 name) const {
    auto it = map_.find(name);
    return it == map_.end() ? nullptr : it->second;
  }
  public: void Register(Content* content) {
    CHECK(!Find(content->name()));
    map_[content->name()] = content;
  }
  public: void ResetForTesting() {
    map_.clear();
  }
  public: static std::vector<Content*> StaticList() {
    return instance()->list();
  }
  public: static v8_glue::Nullable<Content> StaticFind(
      const base::string16& name) {
    return instance()->Find(name);
  }
  public: static void StaticRemove(Content* content) {
    instance()->Unregister(content);
  }
  public: void Unregister(Content* content) {
    auto it = map_.find(content->name());
    if (it == map_.end()) {
      // We called |Content.remove()| for |content|.
      return;
    }
    map_.erase(it);
  }

  DISALLOW_COPY_AND_ASSIGN(ContentList);
};

//////////////////////////////////////////////////////////////////////
//
// ContentClass
//
class ContentClass : public v8_glue::WrapperInfo {
  public: ContentClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~ContentClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto templ = v8_glue::WrapperInfo::CreateConstructorTemplate(isolate);
    return v8_glue::FunctionTemplateBuilder(isolate, templ)
        .SetMethod("find", &ContentList::StaticFind)
        .SetProperty("list", &ContentList::StaticList)
        .SetMethod("remove", &ContentList::StaticRemove)
        .Build();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("name", &Content::name);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Content
//
DEFINE_SCRIPTABLE_OBJECT(Content, ContentClass)

Content::Content(const base::string16& name)
    : name_(name) {
  ContentList::instance()->Register(this);
}

Content::~Content() {
  ContentList::instance()->Unregister(this);
}

}  // namespace contents
}  // namespace dom
