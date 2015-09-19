// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_CLIPBOARD_DATA_TRANSFER_H_
#define EVITA_DOM_CLIPBOARD_DATA_TRANSFER_H_

#include "evita/v8_glue/scriptable.h"

namespace dom {

namespace bindings {
class DataTransferClass;
}

class DataTransferItemList;

class DataTransfer final : public v8_glue::Scriptable<DataTransfer> {
  DECLARE_SCRIPTABLE_OBJECT(DataTransfer)

 public:
  ~DataTransfer() final;

 private:
  friend class bindings::DataTransferClass;

  DataTransfer();

  static DataTransfer* clipboard();
  DataTransferItemList* items();

  DISALLOW_COPY_AND_ASSIGN(DataTransfer);
};

}  // namespace dom

#endif  // EVITA_DOM_CLIPBOARD_DATA_TRANSFER_H_
