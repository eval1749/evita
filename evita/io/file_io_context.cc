// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/file_io_context.h"

#include "base/bind.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
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
                const domapi::OpenFileCallback& callback) {
  CreateFileParams params(mode);
  common::win::scoped_handle handle = ::CreateFileW(file_name.c_str(),
      params.access, params.share_mode, nullptr, params.creation,
      FILE_FLAG_OVERLAPPED, nullptr);
  if (!handle) {
    auto const last_error = ::GetLastError();
    DVLOG(0) << "CreateFileW " << file_name << " error=" << last_error;
    Application::instance()->view_event_handler()->RunCallback(
        base::Bind(callback, domapi::IoContextId(),
                   static_cast<int>(last_error)));
    return INVALID_HANDLE_VALUE;
  }
  return handle.release();
}

void RunCallback(const domapi::FileIoCallback& callback, DWORD num_transferred,
                 DWORD last_error) {
    Application::instance()->view_event_handler()->RunCallback(
        base::Bind(callback, static_cast<int>(num_transferred),
                   static_cast<int>(last_error)));
}
}  // namespace

FileIoContext::FileIoContext(const base::string16& file_name,
                             const base::string16& mode,
                             const domapi::OpenFileCallback& callback)
    : file_handle_(OpenFile(file_name, mode, callback)),
      running_(false) {
  if (!file_handle_.is_valid())
    return;
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
  RunCallback(callback_, bytes_transfered,
              error == ERROR_HANDLE_EOF ? 0 : error);
  if (!file_handle_.is_valid())
    delete this;
}

// io::IoContext
void FileIoContext::Close() {
  if (file_handle_.is_valid()) {
    ::CloseHandle(file_handle_.release());
  }
  if (!running_)
    delete this;
}

void FileIoContext::Read(void* buffer, size_t num_read,
                         const domapi::FileIoCallback& callback) {
if (running_) {
    RunCallback(callback, 0, ERROR_BUSY);
    return;
  }

  running_ = true;
  callback_ = callback;
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
                          const domapi::FileIoCallback& callback) {
  if (running_) {
    RunCallback(callback, 0, ERROR_BUSY);
    return;
  }

  running_ = true;
  callback_ = callback;
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
