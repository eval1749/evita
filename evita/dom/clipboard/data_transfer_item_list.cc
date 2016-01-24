// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "evita/dom/clipboard/data_transfer_item_list.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/clipboard/clipboard.h"
#include "evita/dom/clipboard/data_transfer_item.h"
#include "evita/dom/converter.h"
#include "evita/dom/script_host.h"

namespace gin {
bool Converter<dom::DataTransferData*>::FromV8(v8::Isolate* isolate,
                                               v8::Local<v8::Value> val,
                                               dom::DataTransferData** out) {
  base::string16 string;
  if (ConvertFromV8(isolate, val, &string)) {
    *out = new dom::DataTransferStringData(string);
    return true;
  }

  std::vector<uint8_t> data;
  if (ConvertFromV8(isolate, val, &data)) {
    *out = new dom::DataTransferBlobData(data.data(), data.size());
    return true;
  }

  return false;
}
}  // namespace gin

namespace dom {

DataTransferItemList::DataTransferItemList() : fetched_(false) {}

DataTransferItemList::~DataTransferItemList() {}

int DataTransferItemList::length() const {
  FetchIfNeeded();
  return static_cast<int>(items_.size());
}

void DataTransferItemList::Add(DataTransferData* data,
                               const base::string16& type) {
  FetchIfNeeded();
  Clipboard clipboard;
  if (!clipboard.opened())
    return;
  auto const format = Clipboard::Format::Get(type);
  if (!format) {
    ScriptHost::instance()->ThrowError(
        base::StringPrintf("Unsupported type for clipboard: %ls", type));
    return;
  }
  auto const item = new DataTransferItem(format, data);
  items_.push_back(item);
  clipboard.Add(format, data);
}

void DataTransferItemList::FetchIfNeeded() const {
  if (fetched_)
    return;
  DCHECK(!items_.size());
  Clipboard clipboard;
  if (!clipboard.opened())
    return;
  auto format_code = 0u;
  for (;;) {
    format_code = ::EnumClipboardFormats(format_code);
    if (!format_code)
      break;
    auto const format = Clipboard::Format::Get(format_code);
    if (!format)
      continue;
    auto data = clipboard.Get(format);
    if (!data)
      continue;
    items_.push_back(new DataTransferItem(format, data));
  }
  fetched_ = true;
}

void DataTransferItemList::Clear() {
  Clipboard clipboard;
  if (!clipboard.opened())
    return;
  items_.resize(0);
  clipboard.Clear();
}

DataTransferItem* DataTransferItemList::Get(
    int index,
    ExceptionState* exception_state) const {
  if (ValidateIndex(index, exception_state))
    return nullptr;
  return items_[static_cast<size_t>(index)];
}

void DataTransferItemList::Remove(int index, ExceptionState* exception_state) {
  if (!ValidateIndex(index, exception_state))
    return;
  Clipboard clipboard;
  if (!clipboard.opened())
    return;
  items_.erase(items_.begin() + index);
  ::EmptyClipboard();
  for (auto item : items_) {
    clipboard.Add(item->format(), item->data());
  }
}

bool DataTransferItemList::ValidateIndex(
    int index,
    ExceptionState* exception_state) const {
  FetchIfNeeded();
  if (index >= 0 && index < length())
    return true;
  exception_state->ThrowRangeError(base::StringPrintf(
      "Bad index %d, it must be less than %d", index, length()));
  return false;
}

}  // namespace dom
