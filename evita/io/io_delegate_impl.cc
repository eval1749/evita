// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/io_delegate_impl.h"

#include <unordered_map>

#include "base/bind.h"
#include "base/callback.h"
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/message_loop/message_pump_win.h"
#pragma warning(pop)
#include "base/time/time.h"
#include "evita/dom/public/deferred.h"
#include "evita/dom/public/io_context_id.h"
#include "evita/dom/public/io_error.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/io/file_io_context.h"
#include "evita/io/io_context_utils.h"
#include "evita/io/process_io_context.h"

#define DVLOG_WIN32_ERROR(level, name) \
  DVLOG(level) << name ": " << this << " " << file_name << " err=" << dwError

namespace io {

namespace {
const DWORD kHugeFileSize = 1u << 28;

//////////////////////////////////////////////////////////////////////
//
// QueryFileStatusHandler
//
class QueryFileStatusHandler {
  private: domapi::QueryFileStatusCallbackData data_;
  private: HANDLE find_handle_;

  public: QueryFileStatusHandler(const base::string16& file_name);
  public: ~QueryFileStatusHandler();

  public: const domapi::QueryFileStatusCallbackData& data() const {
    return data_;
  }

  DISALLOW_COPY_AND_ASSIGN(QueryFileStatusHandler);
};

QueryFileStatusHandler::QueryFileStatusHandler(
    const base::string16& file_name) {
  data_ = {0};

  WIN32_FIND_DATAW find_data;
  find_handle_ = ::FindFirstFileW(file_name.c_str(), &find_data);
  if (find_handle_ == INVALID_HANDLE_VALUE) {
    auto const dwError = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "FindFirstFileW");
    data_.error_code = static_cast<int>(dwError);
    return;
  }

  if (find_data.nFileSizeHigh || find_data.nFileSizeLow > kHugeFileSize) {
    data_.error_code = static_cast<int>(ERROR_NOT_ENOUGH_MEMORY);
    return ;
  }

  data_.error_code = 0;
  data_.file_size = static_cast<int>(find_data.nFileSizeLow);
  data_.is_directory = find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
  data_.is_symlink = find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
  data_.last_write_time = base::Time::FromFileTime(find_data.ftLastWriteTime);
  data_.readonly = find_data.dwFileAttributes & FILE_ATTRIBUTE_READONLY;
}

QueryFileStatusHandler::~QueryFileStatusHandler() {
  if (find_handle_ != INVALID_HANDLE_VALUE) {
    ::FindClose(find_handle_);
  }
}

void Resolve(const base::Callback<void(domapi::FileId)>& resolve,
             domapi::FileId context_id) {
  Application::instance()->view_event_handler()->RunCallback(
      base::Bind(resolve , context_id));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// IoDelegateImpl
//
IoDelegateImpl::IoDelegateImpl() {
}

IoDelegateImpl::~IoDelegateImpl() {
}

// domapi::IoDelegate
void IoDelegateImpl::CloseFile(domapi::IoContextId context_id,
                               const domapi::FileIoDeferred& deferred) {
  auto const it = context_map_.find(context_id);
  if (it == context_map_.end()) {
    return;
  }
  it->second->Close(deferred);
  context_map_.erase(it);
}

void IoDelegateImpl::NewProcess(const base::string16& command_line,
                                const domapi::NewProcessDeferred& deferred) {
  auto const process_id = domapi::IoContextId::New();
  auto const process = new ProcessIoContext(process_id, command_line,
                                            deferred);
  context_map_[process_id] = process;
}

void IoDelegateImpl::OpenFile(const base::string16& file_name,
                              const base::string16& mode,
                              const domapi::OpenFileDeferred& deferred) {
  auto file = std::make_unique<FileIoContext>(file_name, mode, deferred);
  if (!file->is_valid())
    return;
  auto const file_id = domapi::IoContextId::New();
  context_map_[file_id] = file.release();
  Resolve(deferred.resolve, domapi::FileId(file_id));
}

void IoDelegateImpl::QueryFileStatus(const base::string16& file_name,
    const domapi::QueryFileStatusCallback& callback) {
  QueryFileStatusHandler handler(file_name);
  Application::instance()->view_event_handler()->RunCallback(
      base::Bind(callback, handler.data()));
}

void IoDelegateImpl::ReadFile(domapi::IoContextId context_id, void* buffer,
                              size_t num_read,
                              const domapi::FileIoDeferred& deferred) {
  auto const it = context_map_.find(context_id);
  if (it == context_map_.end()) {
    Reject(deferred.reject, ERROR_INVALID_HANDLE);
    return;
  }
  it->second->Read(buffer, num_read, deferred);
}

void IoDelegateImpl::WriteFile(domapi::IoContextId context_id, void* buffer,
                               size_t num_write,
                               const domapi::FileIoDeferred& deferred) {
  auto const it = context_map_.find(context_id);
  if (it == context_map_.end()) {
    Reject(deferred.reject, ERROR_INVALID_HANDLE);
    return;
  }
  it->second->Write(buffer, num_write, deferred);
}

}  // namespace io
