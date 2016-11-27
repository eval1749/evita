// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/mock_io_delegate.h"

#include <algorithm>

#include "base/callback.h"
#include "base/logging.h"
#include "evita/dom/public/io_callback.h"
#include "evita/dom/public/io_error.h"
#include "evita/dom/public/promise.h"

namespace dom {

MockIoDelegate::MockIoDelegate()
    : num_close_called_(0), num_remove_called_(0) {}

MockIoDelegate::~MockIoDelegate() {}

void MockIoDelegate::set_bytes(const std::vector<uint8_t> new_bytes) {
  bytes_ = new_bytes;
}

MockIoDelegate::CallResult MockIoDelegate::PopCallResult(
    base::StringPiece name) {
  DCHECK(call_results_.size()) << "Expect " << name;
  auto result = call_results_.front();
  DCHECK_EQ(name, result.name);
  call_results_.pop_front();
  return result;
}

void MockIoDelegate::SetCallResult(base::StringPiece name, int error_code) {
  SetCallResult(name, error_code, 0);
}

void MockIoDelegate::SetCallResult(base::StringPiece name,
                                   int error_code,
                                   int num_transferred) {
  CallResult result;
  result.name = name.as_string();
  result.error_code = error_code;
  result.num_transferred = num_transferred;
  call_results_.push_back(result);
}

void MockIoDelegate::SetComputeFullPathName(base::StringPiece16 result,
                                            int error_code) {
  SetCallResult("ComputeFullPathName", error_code);
  strings_.push_back(result.as_string());
}

void MockIoDelegate::SetFileStatus(const domapi::FileStatus& file_status,
                                   int error_code) {
  SetCallResult("QueryFileStatus", error_code);
  file_status_ = file_status;
}

void MockIoDelegate::SetMakeTempFileName(base::StringPiece16 file_name,
                                         int error_code) {
  SetCallResult("MakeTempFileName", error_code);
  strings_.push_back(file_name.as_string());
}

void MockIoDelegate::SetOpenDirectoryResult(domapi::IoContextId context_id,
                                            int error_code) {
  SetCallResult("OpenDirectory", error_code);
  context_id_.Reset();
  context_id_ = context_id;
}

void MockIoDelegate::SetOpenFileResult(domapi::IoContextId context_id,
                                       int error_code) {
  SetCallResult("OpenFile", error_code);
  context_id_.Reset();
  context_id_ = context_id;
}

void MockIoDelegate::SetOpenResult(base::StringPiece name, int error_code) {
  SetCallResult(name, error_code);
  context_id_.Reset();
  context_id_ = error_code ? domapi::IoContextId() : domapi::IoContextId::New();
}

void MockIoDelegate::SetReadDirectoryResult(
    const std::vector<domapi::FileStatus>& entries) {
  SetCallResult("ReadDirectory", 0);
  directory_entries_ = entries;
}

void MockIoDelegate::SetResourceResult(base::StringPiece operation,
                                       int error_code,
                                       base::StringPiece16 type,
                                       base::StringPiece16 name,
                                       const std::vector<uint8_t>& data) {
  SetCallResult(operation, error_code);
  resource_data_ = data;
  strings_.push_back(name.as_string());
  strings_.push_back(type.as_string());
}

void MockIoDelegate::SetStrings(base::StringPiece name,
                                int error_code,
                                const std::vector<base::string16>& strings) {
  SetCallResult(name, error_code);
  strings_ = strings;
}

// domapi::IoDelegate
void MockIoDelegate::CheckSpelling(const base::string16&,
                                   const CheckSpellingResolver& promise) {
  promise.resolve.Run(check_spelling_result_);
}

void MockIoDelegate::CloseContext(const domapi::IoContextId&,
                                  const domapi::IoIntPromise& resolver) {
  ++num_close_called_;
  auto const result = PopCallResult("CloseContext");
  if (auto const error_code = result.error_code)
    resolver.reject.Run(domapi::IoError(error_code));
  else
    resolver.resolve.Run(true);
}

void MockIoDelegate::ComputeFullPathName(
    const base::string16& path_name,
    const ComputeFullPathNamePromise& promise) {
  const auto& result = PopCallResult("ComputeFullPathName");
  if (auto const error_code = result.error_code) {
    promise.reject.Run(domapi::IoError(error_code));
    return;
  }
  promise.resolve.Run(strings_.back());
  strings_.pop_back();
}

void MockIoDelegate::GetWinResourceNames(
    const domapi::WinResourceId& resource_id,
    const base::string16& type,
    const GetWinResourceNamessPromise& promise) {
  auto const result = PopCallResult("GetWinResourceNames");
  if (const auto error_code = result.error_code)
    return promise.reject.Run(domapi::IoError(error_code));
  promise.resolve.Run(strings_);
}

void MockIoDelegate::GetSpellingSuggestions(
    const base::string16&,
    const GetSpellingSuggestionsResolver& promise) {
  auto const result = PopCallResult("GetSpellingSuggestions");
  if (const auto error_code = result.error_code)
    return promise.reject.Run(error_code);
  promise.resolve.Run(strings_);
}

void MockIoDelegate::LoadWinResource(const domapi::WinResourceId& resource_id,
                                     const base::string16& type,
                                     const base::string16& name,
                                     uint8_t* buffer,
                                     size_t buffer_size,
                                     const domapi::IoIntPromise& promise) {
  auto const result = PopCallResult("LoadWinResource");
  if (auto const error_code = result.error_code)
    return promise.reject.Run(domapi::IoError(error_code));
  const auto expected_type = strings_.back();
  strings_.pop_back();
  const auto expected_name = strings_.back();
  strings_.pop_back();
  if (expected_type != type || expected_name != name)
    return promise.reject.Run(domapi::IoError(ERROR_NOT_FOUND));
  ::memcpy(buffer, resource_data_.data(),
           std::min(resource_data_.size(), buffer_size));
  promise.resolve.Run(static_cast<int>(resource_data_.size()));
}

void MockIoDelegate::MakeTempFileName(
    const base::string16& dir_name,
    const base::string16& prefix,
    const domapi::MakeTempFileNamePromise& resolver) {
  auto const result = PopCallResult("MakeTempFileName");
  if (auto const error_code = result.error_code) {
    resolver.reject.Run(domapi::IoError(error_code));
    return;
  }
  resolver.resolve.Run(dir_name + L"\\" + prefix + strings_.back());
  strings_.pop_back();
}

void MockIoDelegate::MoveFile(const base::string16&,
                              const base::string16&,
                              const domapi::MoveFileOptions& options,
                              const domapi::IoBoolPromise& resolver) {
  auto const result = PopCallResult("MoveFile");
  if (auto const error_code = result.error_code)
    resolver.reject.Run(domapi::IoError(error_code));
  else
    resolver.resolve.Run(options.no_overwrite);
}

void MockIoDelegate::OpenDirectory(
    const base::string16&,
    const domapi::OpenDirectoryPromise& promise) {
  auto const result = PopCallResult("OpenDirectory");
  if (auto const error_code = result.error_code) {
    promise.reject.Run(domapi::IoError(error_code));
    return;
  }
  promise.resolve.Run(domapi::DirectoryId(context_id_));
}

void MockIoDelegate::OpenFile(const base::string16&,
                              const base::string16&,
                              const domapi::OpenFilePromise& promise) {
  auto const result = PopCallResult("OpenFile");
  if (auto const error_code = result.error_code)
    promise.reject.Run(domapi::IoError(error_code));
  else
    promise.resolve.Run(domapi::FileId(context_id_));
}

void MockIoDelegate::OpenProcess(const base::string16&,
                                 const domapi::OpenProcessPromise& promise) {
  auto const result = PopCallResult("OpenFile");
  if (auto const error_code = result.error_code)
    promise.reject.Run(domapi::IoError(error_code));
  else
    promise.resolve.Run(domapi::ProcessId(context_id_));
}

void MockIoDelegate::OpenWinResource(
    const base::string16& file_name,
    const domapi::OpenWinResourcePromise& promise) {
  const auto result = PopCallResult("OpenWinResource");
  if (const auto error_code = result.error_code)
    return promise.reject.Run(domapi::IoError(error_code));
  promise.resolve.Run(domapi::WinResourceId(context_id_));
}

void MockIoDelegate::QueryFileStatus(
    const base::string16&,
    const domapi::QueryFileStatusPromise& promise) {
  auto const result = PopCallResult("QueryFileStatus");
  if (auto const error_code = result.error_code)
    promise.reject.Run(domapi::IoError(error_code));
  else
    promise.resolve.Run(file_status_);
}

void MockIoDelegate::ReadDirectory(
    domapi::IoContextId,
    size_t num_read,
    const domapi::ReadDirectoryPromise& promise) {
  auto const result = PopCallResult("ReadDirectory");
  if (auto const error_code = result.error_code) {
    promise.reject.Run(domapi::IoError(error_code));
    return;
  }
  promise.resolve.Run(directory_entries_);
}

void MockIoDelegate::ReadFile(domapi::IoContextId,
                              void* bytes,
                              size_t num_bytes,
                              const domapi::IoIntPromise& promise) {
  auto const result = PopCallResult("ReadFile");
  if (auto const error_code = result.error_code) {
    promise.reject.Run(domapi::IoError(error_code));
    return;
  }
  promise.resolve.Run(result.num_transferred);
  ::memcpy(bytes, bytes_.data(), std::min(bytes_.size(), num_bytes));
}

void MockIoDelegate::RemoveFile(const base::string16&,
                                const domapi::IoBoolPromise& resolver) {
  ++num_remove_called_;
  auto const result = PopCallResult("RemoveFile");
  if (auto const error_code = result.error_code)
    resolver.reject.Run(domapi::IoError(error_code));
  else
    resolver.resolve.Run(true);
}

void MockIoDelegate::WriteFile(domapi::IoContextId,
                               void* bytes,
                               size_t num_bytes,
                               const domapi::IoIntPromise& promise) {
  auto const result = PopCallResult("WriteFile");
  if (auto const error_code = result.error_code) {
    promise.reject.Run(domapi::IoError(error_code));
    return;
  }
  bytes_.resize(num_bytes);
  ::memcpy(&bytes_[0], bytes, num_bytes);
  promise.resolve.Run(result.num_transferred);
}

}  // namespace dom
