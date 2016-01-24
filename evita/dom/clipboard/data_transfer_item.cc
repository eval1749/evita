// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/clipboard/data_transfer_item.h"

#include "base/strings/stringprintf.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/converter.h"

namespace gin {
v8::Local<v8::Value> Converter<dom::DataTransferData::Kind>::ToV8(
    v8::Isolate* isolate,
    dom::DataTransferData::Kind kind) {
  return StringToV8(isolate, dom::DataTransferData::KindToString(kind));
}
}  // namespace gin

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// DataTransferItem
//
DataTransferItem::DataTransferItem(const Clipboard::Format* format,
                                   DataTransferData* data)
    : data_(data), format_(format) {}

DataTransferItem::DataTransferItem(const base::string16& type,
                                   DataTransferData* data)
    : DataTransferItem(Clipboard::Format::Get(type), data) {}

DataTransferItem::~DataTransferItem() {}

DataTransferData::Kind DataTransferItem::kind() const {
  return data_->kind();
}

const base::string16& DataTransferItem::type() const {
  return format_->mime_type();
}

std::vector<uint8_t> DataTransferItem::GetAsBlob(
    ExceptionState* exception_state) const {
  if (kind() != DataTransferData::Kind::Blob) {
    exception_state->ThrowError(
        base::StringPrintf("%ls isn't blob.", type().c_str()));
    return std::vector<uint8_t>();
  }
  std::vector<uint8_t> data(data_->num_bytes());
  ::memcpy(&data[0], data_->bytes(), data_->num_bytes());
  return data;
}

base::string16 DataTransferItem::GetAsString(
    ExceptionState* exception_state) const {
  if (kind() != DataTransferData::Kind::String) {
    exception_state->ThrowError(
        base::StringPrintf("%ls isn't string.", type().c_str()));
    return base::string16();
  }
  base::string16 data(data_->num_bytes() / 2, 0);
  ::memcpy(&data[0], data_->bytes(), data_->num_bytes());
  return data;
}

}  // namespace dom
