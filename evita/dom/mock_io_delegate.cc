// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/mock_io_delegate.h"

#include "evita/dom/public/io_callback.h"

#include "base/callback.h"

namespace dom {

MockIoDelegate::MockIoDelegate() : error_code_(0), io_handle_(nullptr) {
}

MockIoDelegate::~MockIoDelegate() {
}

void MockIoDelegate::SetOpenFileCallbackData(domapi::IoHandle* io_handle,
                                             int error_code) {
  io_handle_ = io_handle;
  error_code_ = error_code;
}

void MockIoDelegate::SetQueryFileStatusCallbackData(
    const domapi::QueryFileStatusCallbackData& data) {
  data_ = data;
}

// domapi::IoDelegate
void MockIoDelegate::OpenFile(const base::string16&,
    const base::string16&, const OpenFileCallback& callback) {
  callback.Run(io_handle_, error_code_);
}

void MockIoDelegate::QueryFileStatus(const base::string16&,
    const QueryFileStatusCallback& callback) {
  callback.Run(data_);
}

}  // namespace dom
