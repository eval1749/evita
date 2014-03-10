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
  error_code_ = error_code;
  num_transferred_ = num_transferred;
}

void MockIoDelegate::SetOpenFileDeferredData(domapi::IoContextId context_id,
                                             int error_code) {
  context_id_ = context_id;
  error_code_ = error_code;
}

void MockIoDelegate::SetFileStatus(const domapi::FileStatus& file_status,
                                   int error_code) {
  error_code_ = error_code;
  file_status_ = file_status;
}

// domapi::IoDelegate
void MockIoDelegate::OpenFile(const base::string16&,
    const base::string16&, const domapi::OpenFileDeferred& deferred) {
  if (error_code_)
    deferred.reject.Run(domapi::IoError(error_code_));
  else
    deferred.resolve.Run(domapi::FileId(domapi::IoContextId::New()));
}

void MockIoDelegate::OpenProcess(const base::string16&,
    const domapi::OpenProcessDeferred& deferred) {
  if (error_code_)
    deferred.reject.Run(domapi::IoError(error_code_));
  else
    deferred.resolve.Run(domapi::ProcessId(domapi::IoContextId::New()));
}

void MockIoDelegate::QueryFileStatus(const base::string16&,
    const domapi::QueryFileStatusDeferred& deferred) {
  if (error_code_)
    deferred.reject.Run(domapi::IoError(error_code_));
  else
    deferred.resolve.Run(file_status_);
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
