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
#include "common/win/scoped_handle.h"
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

class scoped_find_handle {
  private: HANDLE handle_;

  public: scoped_find_handle(HANDLE handle) : handle_(handle) {
  }

  public: ~scoped_find_handle() {
    if (is_valid())
      ::FindClose(handle_);
  }

  public: bool is_valid() const { return handle_ != INVALID_HANDLE_VALUE; }

  DISALLOW_COPY_AND_ASSIGN(scoped_find_handle);
};

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

void IoDelegateImpl::OpenProcess(const base::string16& command_line,
                                const domapi::OpenProcessDeferred& deferred) {
  auto const process_id = domapi::IoContextId::New();
  auto const process = new ProcessIoContext(process_id, command_line,
                                            deferred);
  context_map_[process_id] = process;
}

void IoDelegateImpl::QueryFileStatus(const base::string16& file_name,
    const domapi::QueryFileStatusDeferred& deferred) {
  WIN32_FIND_DATAW find_data;
  scoped_find_handle find_handle = ::FindFirstFileW(file_name.c_str(),
                                                    &find_data);
  if (!find_handle.is_valid()) {
    auto const last_error = ::GetLastError();
    DVLOG(0) << "FindFirstFileW error=" << last_error;
    Reject(deferred.reject, last_error);
    return;
  }

  if (find_data.nFileSizeHigh || find_data.nFileSizeLow > kHugeFileSize) {
    Reject(deferred.reject, ERROR_NOT_ENOUGH_MEMORY);
    return;
  }

  domapi::FileStatus data = {0};
  data.file_size = static_cast<int>(find_data.nFileSizeLow);
  data.is_directory = find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
  data.is_symlink = find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
  data.last_write_time = base::Time::FromFileTime(find_data.ftLastWriteTime);
  data.readonly = find_data.dwFileAttributes & FILE_ATTRIBUTE_READONLY;

  Application::instance()->view_event_handler()->RunCallback(
      base::Bind(deferred.resolve , data));
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
