// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_CLIPBOARD_DATA_TRANSFER_ITEM_LIST_H_
#define EVITA_DOM_CLIPBOARD_DATA_TRANSFER_ITEM_LIST_H_

#include <vector>

#include "base/basictypes.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class DataTransferData;
class DataTransferItem;

namespace bindings {
class DataTransferItemListClass;
}

class DataTransferItemList final
    : public v8_glue::Scriptable<DataTransferItemList> {
  DECLARE_SCRIPTABLE_OBJECT(DataTransferItemList)
 public:
  DataTransferItemList();
  ~DataTransferItemList() final;

 private:
  friend class bindings::DataTransferItemListClass;

  int length() const;

  void Add(DataTransferData* data, const base::string16& type);
  void Clear();
  void FetchIfNeeded() const;
  DataTransferItem* Get(int index) const;
  void Remove(int index);
  bool ValidateIndex(int index) const;

  mutable bool fetched_;
  mutable std::vector<DataTransferItem*> items_;

  DISALLOW_COPY_AND_ASSIGN(DataTransferItemList);
};

}  // namespace dom

namespace gin {
template <>
struct Converter<dom::DataTransferData*> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Handle<v8::Value> val,
                     dom::DataTransferData** out);
};
}  // namespace gin

#endif  // EVITA_DOM_CLIPBOARD_DATA_TRANSFER_ITEM_LIST_H_
