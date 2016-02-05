// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_CLIPBOARD_DATA_TRANSFER_ITEM_LIST_H_
#define EVITA_DOM_CLIPBOARD_DATA_TRANSFER_ITEM_LIST_H_

#include <vector>

#include "base/macros.h"
#include "evita/ginx/scriptable.h"

namespace dom {

class DataTransferData;
class DataTransferItem;
class ExceptionState;

namespace bindings {
class DataTransferItemListClass;
}

//////////////////////////////////////////////////////////////////////
//
// DataTransferItemList
//
class DataTransferItemList final
    : public ginx::Scriptable<DataTransferItemList> {
  DECLARE_SCRIPTABLE_OBJECT(DataTransferItemList)
 public:
  DataTransferItemList();
  ~DataTransferItemList() final;

 private:
  friend class bindings::DataTransferItemListClass;

  // bindings
  int length(ExceptionState* exception_state) const;

  void Add(DataTransferData* data,
           const base::string16& type,
           ExceptionState* exception_state);
  void Clear(ExceptionState* exception_state);
  void FetchIfNeeded(ExceptionState* exception_state) const;
  DataTransferItem* Get(int index, ExceptionState* exception_state) const;
  void Remove(int index, ExceptionState* exception_state);
  bool ValidateIndex(int index, ExceptionState* exception_state) const;

  mutable bool fetched_;
  mutable std::vector<DataTransferItem*> items_;

  DISALLOW_COPY_AND_ASSIGN(DataTransferItemList);
};

}  // namespace dom

namespace gin {
template <>
struct Converter<dom::DataTransferData*> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     dom::DataTransferData** out);
};
}  // namespace gin

#endif  // EVITA_DOM_CLIPBOARD_DATA_TRANSFER_ITEM_LIST_H_
