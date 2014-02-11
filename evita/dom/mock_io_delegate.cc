// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/mock_io_delegate.h"

#include "evita/dom/public/io_callback.h"

#include "base/callback.h"

namespace dom {

MockIoDelegate::MockIoDelegate() {
}

MockIoDelegate::~MockIoDelegate() {
}

void MockIoDelegate::SetQueryFileStatusCallbackData(
    const domapi::QueryFileStatusCallbackData& data) {
  data_ = data;
}

// domapi::IoDelegate
void MockIoDelegate::QueryFileStatus(const base::string16&,
    const QueryFileStatusCallback& callback) {
  callback.Run(data_);
}

}  // namespace dom
