// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/clipboard/data_transfer.h"

#include "base/strings/string16.h"
#include "evita/dom/clipboard/data_transfer_item_list.h"
#include "evita/dom/script_host.h"
#include "evita/v8_glue/function_template_builder.h"

namespace dom {

DataTransfer::DataTransfer() {
}

DataTransfer::~DataTransfer() {
}

DataTransfer* DataTransfer::clipboard() {
  return new DataTransfer();
}

DataTransferItemList* DataTransfer::items() {
  return new DataTransferItemList();
}

}  // namespace dom
