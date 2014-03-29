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

class DataTransferItemList : public v8_glue::Scriptable<DataTransferItemList> {
  DECLARE_SCRIPTABLE_OBJECT(DataTransferItemList)

  private: mutable bool fetched_;
  private: mutable std::vector<DataTransferItem*> items_;

  public: DataTransferItemList();
  public: virtual ~DataTransferItemList();

  public: int length() const;

  public: void Add(DataTransferData* data, const base::string16& type);
  public: void Clear();
  private: void FetchIfNeeded() const;
  public: DataTransferItem* Get(int index) const;
  public: void Remove(int index);
  private: bool ValidateIndex(int index) const;

  DISALLOW_COPY_AND_ASSIGN(DataTransferItemList);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_clipboard_data_transfer_item_list_h)
