// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_clipboard_data_transfer_item_list_h)
#define INCLUDE_evita_dom_clipboard_data_transfer_item_list_h

#include "base/basictypes.h"
#include "evita/v8_glue/scriptable.h"

#include <vector>

namespace dom {

class DataTransferData;
class DataTransferItem;

namespace bindings {
class DataTransferItemListClass;
}

class DataTransferItemList : public v8_glue::Scriptable<DataTransferItemList> {
  DECLARE_SCRIPTABLE_OBJECT(DataTransferItemList)
  friend class bindings::DataTransferItemListClass;

  private: mutable bool fetched_;
  private: mutable std::vector<DataTransferItem*> items_;

  public: DataTransferItemList();
  public: virtual ~DataTransferItemList();

  private: int length() const;

  private: void Add(DataTransferData* data, const base::string16& type);
  private: void Clear();
  private: void FetchIfNeeded() const;
  private: DataTransferItem* Get(int index) const;
  private: void Remove(int index);
  private: bool ValidateIndex(int index) const;

  DISALLOW_COPY_AND_ASSIGN(DataTransferItemList);
};

}  // namespace dom

namespace gin {
template<>
struct Converter<dom::DataTransferData*> {
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     dom::DataTransferData** out);
};
}  // namespace gin

#endif //!defined(INCLUDE_evita_dom_clipboard_data_transfer_item_list_h)
