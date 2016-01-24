// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_CLIPBOARD_DATA_TRANSFER_ITEM_H_
#define EVITA_DOM_CLIPBOARD_DATA_TRANSFER_ITEM_H_

#include <memory>
#include <string>
#include <vector>

#include "evita/dom/clipboard/clipboard.h"
#include "evita/dom/clipboard/data_transfer_data.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

namespace bindings {
class DataTransferItemClass;
}

class ExceptionState;

//////////////////////////////////////////////////////////////////////
//
// DataTransferItem
//
class DataTransferItem final : public v8_glue::Scriptable<DataTransferItem> {
  DECLARE_SCRIPTABLE_OBJECT(DataTransferItem)
 public:
  DataTransferItem(const Clipboard::Format* format, DataTransferData* data);
  DataTransferItem(const base::string16& type, DataTransferData* data);
  ~DataTransferItem() final;

  const DataTransferData* data() const { return data_.get(); }
  const Clipboard::Format* format() const { return format_; }
  DataTransferData::Kind kind() const;
  const base::string16& type() const;

 private:
  friend class bindings::DataTransferItemClass;

  // bindings
  std::vector<uint8_t> GetAsBlob(ExceptionState* exception_state) const;
  base::string16 GetAsString(ExceptionState* exception_state) const;

  const std::unique_ptr<DataTransferData> data_;
  const Clipboard::Format* const format_;

  DISALLOW_COPY_AND_ASSIGN(DataTransferItem);
};

}  // namespace dom

namespace gin {
template <>
struct Converter<dom::DataTransferData::Kind> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   dom::DataTransferData::Kind kind);
};
}  // namespace gin

#endif  // EVITA_DOM_CLIPBOARD_DATA_TRANSFER_ITEM_H_
