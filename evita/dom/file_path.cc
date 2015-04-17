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

//////////////////////////////////////////////////////////////////////
//
// FilePathWrapperInfo
//
class FilePathWrapperInfo : public v8_glue::WrapperInfo {
  public: FilePathWrapperInfo(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~FilePathWrapperInfo() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto templ = v8_glue::WrapperInfo::CreateConstructorTemplate(isolate);
    return v8_glue::FunctionTemplateBuilder(isolate, templ)
        .SetMethod("fullPath", &FilePath::FullPath)
        .Build();
  }

  DISALLOW_COPY_AND_ASSIGN(FilePathWrapperInfo);
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FilePath
//
DEFINE_SCRIPTABLE_OBJECT(FilePath, FilePathWrapperInfo);

FilePath::FilePath() {
}

FilePath::~FilePath() {
}

base::string16 FilePath::FullPath(const base::string16& file_name) {
  base::string16 full_name(MAX_PATH + 1, 0);
  base::char16* file_start = nullptr;
  auto const length = ::GetFullPathNameW(
      file_name.c_str(),
      static_cast<DWORD>(full_name.length()),
      &full_name[0],
      &file_start);
  full_name.resize(length);
  return full_name;
}

}  // namespace dom
