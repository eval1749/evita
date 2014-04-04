// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/clipboard/data_transfer.h"

#include "base/strings/string16.h"
#include "evita/dom/clipboard/data_transfer_item_list.h"
#include "evita/dom/script_host.h"
#include "evita/v8_glue/function_template_builder.h"

namespace dom {

namespace {

//////////////////////////////////////////////////////////////////////
//
// DataTransferClass
//
class DataTransferClass : public v8_glue::WrapperInfo {
  public: DataTransferClass(const char* name);
  public: virtual ~DataTransferClass();

  private: static DataTransfer* clipboard();

  private: static DataTransfer* NewDataTransfer();

  // v8_glue::WrapperInfo
  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override;
  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override;

  DISALLOW_COPY_AND_ASSIGN(DataTransferClass);
};

DataTransferClass::DataTransferClass(const char* name)
    : v8_glue::WrapperInfo(name) {
}

DataTransferClass::~DataTransferClass() {
}

DataTransfer* DataTransferClass::clipboard() {
  return new DataTransfer();
}

DataTransfer* DataTransferClass::NewDataTransfer() {
  ScriptHost::instance()->ThrowError("Can't create DataTransfer.");
  return nullptr;
}

// v8_glue::WrapperInfo
v8::Handle<v8::FunctionTemplate> DataTransferClass::CreateConstructorTemplate(
    v8::Isolate* isolate) {
  auto templ = v8_glue::CreateConstructorTemplate(isolate,
      &DataTransferClass::NewDataTransfer);
  return v8_glue::FunctionTemplateBuilder(isolate, templ)
      .SetProperty("clipboard", &DataTransferClass::clipboard)
      .Build();
}

void DataTransferClass:: SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) {
  builder
      .SetProperty("items", &DataTransfer::items);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// DataTransfer
//
DEFINE_SCRIPTABLE_OBJECT(DataTransfer, DataTransferClass);

DataTransfer::DataTransfer() {
}

DataTransfer::~DataTransfer() {
}

DataTransferItemList* DataTransfer::items() {
  return new DataTransferItemList();
}

}  // namespace dom
