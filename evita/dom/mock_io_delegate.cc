// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/mock_io_delegate.h"

#include "base/callback.h"
#include "evita/dom/public/deferred.h"
#include "evita/dom/public/io_callback.h"
#include "evita/dom/public/io_error.h"

namespace dom {

MockIoDelegate::MockIoDelegate() {
}

MockIoDelegate::~MockIoDelegate() {
}

int MockIoDelegate::PopErrorCode() {
  auto const error_code = error_codes_.front();
  error_codes_.pop_front();
  return error_code;
}

void MockIoDelegate::SetFileIoDeferredData(int num_transferred,
                                           int error_code) {
  error_codes_.push_back(error_code);
  num_transferreds_.push_back(num_transferred);
}

void MockIoDelegate::SetIoResult(int error_code) {
  error_codes_.push_back(error_code);
}

void MockIoDelegate::SetMakeTempFileName(const base::string16 file_name,
                                         int error_code) {
  error_codes_.push_back(error_code);
  temp_file_name_ = file_name;
}

void MockIoDelegate::SetOpenFileDeferredData(domapi::IoContextId context_id,
                                             int error_code) {
  context_id_ = context_id;
  error_codes_.push_back(error_code);
}

void MockIoDelegate::SetFileStatus(const domapi::FileStatus& file_status,
                                   int error_code) {
  error_codes_.push_back(error_code);
  file_status_ = file_status;
}

// domapi::IoDelegate
void MockIoDelegate::MakeTempFileName(
      const base::string16& dir_name, const base::string16& prefix,
      const domapi::MakeTempFileNameResolver& resolver) {
  if (auto const error_code = PopErrorCode())
    resolver.reject.Run(domapi::IoError(error_code));
  else
    resolver.resolve.Run(dir_name + L"\\" + prefix + temp_file_name_);
}

void MockIoDelegate::MoveFile(const base::string16&,
                              const base::string16&,
                              const domapi::MoveFileOptions& options,
                              const domapi::IoResolver& resolver) {
  if (auto const error_code = PopErrorCode())
    resolver.reject.Run(domapi::IoError(error_code));
  else
    resolver.resolve.Run(options.no_overwrite);
}

void MockIoDelegate::OpenFile(const base::string16&,
    const base::string16&, const domapi::OpenFileDeferred& deferred) {
  if (auto const error_code = PopErrorCode())
    deferred.reject.Run(domapi::IoError(error_code));
  else
    deferred.resolve.Run(domapi::FileId(domapi::IoContextId::New()));
}

void MockIoDelegate::OpenProcess(const base::string16&,
    const domapi::OpenProcessDeferred& deferred) {
  if (auto const error_code = PopErrorCode())
    deferred.reject.Run(domapi::IoError(error_code));
  else
    deferred.resolve.Run(domapi::ProcessId(domapi::IoContextId::New()));
}

void MockIoDelegate::QueryFileStatus(const base::string16&,
    const domapi::QueryFileStatusDeferred& deferred) {
  if (auto const error_code = PopErrorCode())
    deferred.reject.Run(domapi::IoError(error_code));
  else
    deferred.resolve.Run(file_status_);
}

void MockIoDelegate::ReadFile(domapi::IoContextId, void*, size_t,
                              const domapi::FileIoDeferred& deferred) {
  if (auto const error_code = PopErrorCode()) {
    deferred.reject.Run(domapi::IoError(error_code));
    return;
  }
  auto const num_transferred = num_transferreds_.front();
  num_transferreds_.pop_front();
  deferred.resolve.Run(num_transferred);
}

void MockIoDelegate::RemoveFile(const base::string16&,
                                const domapi::IoResolver& resolver) {
  if (auto const error_code = PopErrorCode())
    resolver.reject.Run(domapi::IoError(error_code));
  else
    resolver.resolve.Run(true);
}

void MockIoDelegate::WriteFile(domapi::IoContextId, void*, size_t,
                              const domapi::FileIoDeferred& deferred) {
  if (auto const error_code = PopErrorCode()) {
    deferred.reject.Run(domapi::IoError(error_code));
    return;
  }
  auto const num_transferred = num_transferreds_.front();
  num_transferreds_.pop_front();
  deferred.resolve.Run(num_transferred);
}

}  // namespace dom
