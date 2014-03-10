// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/mock_io_delegate.h"

#include "base/callback.h"
#include "evita/dom/public/deferred.h"
#include "evita/dom/public/io_callback.h"
#include "evita/dom/public/io_error.h"

namespace dom {

MockIoDelegate::MockIoDelegate()
    : error_code_(0), num_transferred_(0) {
}

MockIoDelegate::~MockIoDelegate() {
}

void MockIoDelegate::SetFileIoDeferredData(int num_transferred,
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
void MockIoDelegate::NewProcess(const base::string16&,
    const domapi::NewProcessCallback& callback) {
  callback.Run(context_id_, error_code_);
}

void MockIoDelegate::OpenFile(const base::string16&,
    const base::string16&, const domapi::OpenFileCallback& callback) {
  callback.Run(context_id_, error_code_);
}

void MockIoDelegate::QueryFileStatus(const base::string16&,
    const domapi::QueryFileStatusCallback& callback) {
  callback.Run(data_);
}

void MockIoDelegate::ReadFile(domapi::IoContextId, void*, size_t,
                              const domapi::FileIoDeferred& deferred) {
  if (error_code_)
    deferred.reject.Run(domapi::IoError(error_code_));
  else
    deferred.resolve.Run(num_transferred_);
}

void MockIoDelegate::WriteFile(domapi::IoContextId, void*, size_t,
                              const domapi::FileIoDeferred& deferred) {
  if (error_code_)
    deferred.reject.Run(domapi::IoError(error_code_));
  else
    deferred.resolve.Run(num_transferred_);
}

}  // namespace dom
