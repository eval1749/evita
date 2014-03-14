// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/mutation_record.h"

#include "evita/dom/converter.h"
#include "evita/dom/document.h"
#include "evita/v8_glue/per_isolate_data.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// MutationRecordClass
//
class MutationRecordClass : public v8_glue::WrapperInfo {
  public: MutationRecordClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~MutationRecordClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto const templ = v8::FunctionTemplate::New(isolate);
    templ->SetCallHandler(NewMutationRecord);
    return templ;
  }

  private: static void NewMutationRecord(
      const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto const isolate = info.GetIsolate();
    auto const data = v8_glue::PerIsolateData::From(isolate);
    if (data->is_creating_wrapper())
      return;
    isolate->ThrowException(
      gin::StringToV8(isolate, info.IsConstructCall() ?
          "Cannot use with new operator" : "Cannot be clled as function"));
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("document", &MutationRecord::document)
        .SetProperty("offset", &MutationRecord::offset)
        .SetProperty("type", &MutationRecord::type);
  }

  DISALLOW_COPY_AND_ASSIGN(MutationRecordClass);
};
}   // namespace

//////////////////////////////////////////////////////////////////////
//
// MutationRecord
//
DEFINE_SCRIPTABLE_OBJECT(MutationRecord, MutationRecordClass);

MutationRecord::MutationRecord(const base::string16& type, Document* document,
                               text::Posn offset)
    : document_(document), offset_(offset), type_(type) {
}

MutationRecord::~MutationRecord() {
}

}  // namespace dom
