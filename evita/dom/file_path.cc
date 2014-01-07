// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/file_path.h"

#include "base/strings/string16.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {

base::string16 FullPath(const base::string16& filename) {
  base::string16 full_name(MAX_PATH + 1, 0);
  char16* file_start = nullptr;
  auto const length = ::GetFullPathNameW(
      filename.c_str(),
      full_name.length(),
      &full_name[0],
      &file_start);
  full_name.resize(length);
  return full_name;
}

//////////////////////////////////////////////////////////////////////
//
// FilePathWrapperInfo
//
class FilePathWrapperInfo : public v8_glue::WrapperInfo {
  public: FilePathWrapperInfo() : v8_glue::WrapperInfo("FilePath") {
  }
  public: ~FilePathWrapperInfo() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto templ = v8_glue::WrapperInfo::CreateConstructorTemplate(isolate);
    return v8_glue::FunctionTemplateBuilder(isolate, templ)
        .SetMethod("fullPath", &FullPath)
        .Build();
  }
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FilePath
//
FilePath::FilePath() {
}

FilePath::~FilePath() {
}

v8_glue::WrapperInfo* FilePath::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(FilePathWrapperInfo, wrapper_info, ());
  return &wrapper_info;
}

}  // namespace dom
