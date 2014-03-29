// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_clipboard_data_transfer_item_h)
#define INCLUDE_evita_dom_clipboard_data_transfer_item_h

#include <memory>

#include "evita/dom/clipboard/clipboard.h"
#include "evita/dom/clipboard/data_transfer_data.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class DataTransferItem : public v8_glue::Scriptable<DataTransferItem> {
  DECLARE_SCRIPTABLE_OBJECT(DataTransferItem)

  private: const std::unique_ptr<DataTransferData> data_;
  private: const Clipboard::Format* const format_;

  public: DataTransferItem(const Clipboard::Format* format,
                           DataTransferData* data);
  public: DataTransferItem(const base::string16& type, DataTransferData* data);
  public: virtual ~DataTransferItem();

  public: const DataTransferData* data() const { return data_.get(); }
  public: const Clipboard::Format* format() const { return format_; }
  public: DataTransferData::Kind kind() const;
  public: const base::string16& type() const;

  public: std::vector<uint8_t> GetAsBlob() const;
  public: base::string16 GetAsString() const;

  DISALLOW_COPY_AND_ASSIGN(DataTransferItem);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_clipboard_data_transfer_item_h)
