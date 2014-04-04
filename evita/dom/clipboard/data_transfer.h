// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_clipboard_data_transfer_h)
#define INCLUDE_evita_dom_clipboard_data_transfer_h

#include "evita/v8_glue/scriptable.h"

namespace dom {

class DataTransferItemList;

class DataTransfer : public v8_glue::Scriptable<DataTransfer> {
  DECLARE_SCRIPTABLE_OBJECT(DataTransfer)

  public: DataTransfer();
  public: virtual ~DataTransfer();

  public: DataTransferItemList* items();

  DISALLOW_COPY_AND_ASSIGN(DataTransfer);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_clipboard_data_transfer_h)
