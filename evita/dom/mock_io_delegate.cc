// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/mock_io_delegate.h"

#include "evita/dom/public/io_callback.h"

#include "base/callback.h"

namespace dom {

MockIoDelegate::MockIoDelegate()
    : error_code_(0), num_transferred_(0) {
}

MockIoDelegate::~MockIoDelegate() {
}

void MockIoDelegate::SetFileIoCallbackData(int num_transferred,
                                           int error_code) {
  num_transferred_ = num_transferred;
  error_code_ = error_code;
}

void MockIoDelegate::SetOpenFileCallbackData(domapi::IoContextId context_id,
                                             int error_code) {
  context_id_ = context_id;
  error_code_ = error_code;
}

void MockIoDelegate::SetQueryFileStatusCallbackData(
    const domapi::QueryFileStatusCallbackData& data) {
  data_ = data;
}

// domapi::IoDelegate
void MockIoDelegate::OpenFile(const base::string16&,
    const base::string16&, const OpenFileCallback& callback) {
  callback.Run(context_id_, error_code_);
}

void MockIoDelegate::QueryFileStatus(const base::string16&,
    const QueryFileStatusCallback& callback) {
  callback.Run(data_);
}

void MockIoDelegate::ReadFile(domapi::IoContextId, void*, size_t,
                              const FileIoCallback& callback) {
  callback.Run(num_transferred_, error_code_);
}

void MockIoDelegate::WriteFile(domapi::IoContextId, void*, size_t,
                              const FileIoCallback& callback) {
  callback.Run(num_transferred_, error_code_);
}

}  // namespace dom
