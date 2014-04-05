// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/file_io_context.h"

#include "base/bind.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/io/io_context_utils.h"
#include "evita/io/io_manager.h"

namespace io {

namespace {
//////////////////////////////////////////////////////////////////////
//
// CreateFileParams
//
struct CreateFileParams {
  DWORD access;
  DWORD creation;
  DWORD share_mode;

  CreateFileParams(const base::string16& mode) {
    share_mode = FILE_SHARE_DELETE | FILE_SHARE_READ;

    if (mode.empty() || mode[0] != 'w') {
      access = GENERIC_READ;
      creation = OPEN_EXISTING;
      return;
    }
    access = GENERIC_WRITE;
    creation = CREATE_ALWAYS | TRUNCATE_EXISTING;
  }
};

HANDLE OpenFile(const base::string16& file_name,
                const base::string16& mode,
                const domapi::OpenFileDeferred& deferred) {
  CreateFileParams params(mode);
  common::win::scoped_handle handle = ::CreateFileW(file_name.c_str(),
      params.access, params.share_mode, nullptr, params.creation,
      FILE_FLAG_OVERLAPPED, nullptr);
  if (!handle) {
    auto const last_error = ::GetLastError();
    DVLOG(0) << "CreateFileW " << file_name << " error=" << last_error;
    Reject(deferred.reject, last_error);
    return INVALID_HANDLE_VALUE;
  }
  return handle.release();
}

}  // namespace

void Resolve(const base::Callback<void(domapi::FileId)>& resolve,
             domapi::FileId context_id) {
  Application::instance()->view_event_handler()->RunCallback(
      base::Bind(resolve , context_id));
}

FileIoContext::FileIoContext(const base::string16& file_name,
                             const base::string16& mode,
                             const domapi::OpenFileDeferred& deferred)
    : file_handle_(OpenFile(file_name, mode, deferred)),
      running_(false) {
  if (!file_handle_.is_valid())
    return;
  overlapped = {0};
  handler = this;
  Application::instance()->GetIoManager()->RegisterIoHandler(
      file_handle_.get(), this);
}

FileIoContext::~FileIoContext() {
}

// base::MessagePumpForIO::FileIoContext
void FileIoContext::OnIOCompleted(IOContext*,
                                  DWORD bytes_transfered,
                                  DWORD error) {
  overlapped.Offset += bytes_transfered;
  running_ = false;
  if (!error || error == ERROR_HANDLE_EOF)
    Resolve(deferred_.resolve, bytes_transfered);
  else
    Reject(deferred_.reject, error);
  if (!file_handle_.is_valid())
    delete this;
}

// io::IoContext
void FileIoContext::Close(const domapi::FileIoDeferred& deferred) {
  if (file_handle_.is_valid()) {
    if (!::CloseHandle(file_handle_.get())) {
      auto const last_error = ::GetLastError();
      DVLOG(0) << "CloseHandle error=" << last_error;
      Reject(deferred.reject, last_error);
      return;
    }
    file_handle_.release();
  }
  if (!running_)
    delete this;
  Resolve(deferred.resolve, 0u);
}

void FileIoContext::Read(void* buffer, size_t num_read,
                         const domapi::FileIoDeferred& deferred) {
if (running_) {
    Reject(deferred.reject, ERROR_BUSY);
    return;
  }

  running_ = true;
  deferred_ = deferred;
  DWORD read;
  auto const succeeded = ::ReadFile(file_handle_.get(), buffer, num_read,
                                    &read, &overlapped);
  DCHECK(!succeeded);

  auto const error = ::GetLastError();
  if (error == ERROR_IO_PENDING)
    return;
  running_ = false;
  OnIOCompleted(this, 0, error);
}

void FileIoContext::Write(void* buffer, size_t num_write,
                          const domapi::FileIoDeferred& deferred) {
  if (running_) {
    Reject(deferred.reject, ERROR_BUSY);
    return;
  }

  running_ = true;
  deferred_ = deferred;
  DWORD written;
  auto const succeeded = ::WriteFile(file_handle_.get(), buffer, num_write,
                                     &written, &overlapped);
  DCHECK(!succeeded);
  auto const error = ::GetLastError();
  if (error == ERROR_IO_PENDING)
    return;
  running_ = false;
  OnIOCompleted(this, 0, error);
}

}  // namespace io
