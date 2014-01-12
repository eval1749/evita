// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/console.h"

#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
void Log(const std::vector<base::string16>& messages) {
  auto const document = Document::GetOrNew(L"*console log*");
  auto const buffer = document->buffer();
  for (auto message : messages) {
    buffer->Insert(buffer->GetEnd(), message.c_str());
  }
  buffer->Insert(buffer->GetEnd(), L"\n");
}

//////////////////////////////////////////////////////////////////////
//
// ConsoleWrapperInfo
//
class ConsoleWrapperInfo : public v8_glue::WrapperInfo {
  public: ConsoleWrapperInfo(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~ConsoleWrapperInfo() = default;

  public: virtual AbstractScriptable* singleton() const override {
    return Console::instance();
  }

  public: virtual const char* singleton_name() const override {
    return "console";
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder.SetMethod("log", Log);
  }
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Console
//
DEFINE_SCRIPTABLE_OBJECT(Console, ConsoleWrapperInfo);

Console::Console() {
}

Console::~Console() {
}

}  // namespace dom
