// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/file_io_context.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/trace_event/trace_event.h"
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

  explicit CreateFileParams(const base::string16& mode) {
    share_mode = FILE_SHARE_DELETE | FILE_SHARE_READ;

    if (mode.empty() || mode[0] != 'w') {
      access = GENERIC_READ;
      creation = OPEN_EXISTING;
      return;
    }
    access = GENERIC_WRITE;
    creation = CREATE_ALWAYS;
  }
};

HANDLE OpenFile(const base::string16& file_name,
                const base::string16& mode,
                const domapi::OpenFileDeferred& deferred) {
  CreateFileParams params(mode);
  common::win::scoped_handle handle(
      ::CreateFileW(file_name.c_str(), params.access, params.share_mode,
                    nullptr, params.creation, FILE_FLAG_OVERLAPPED, nullptr));
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
  editor::Application::instance()->view_event_handler()->RunCallback(
      base::Bind(resolve, context_id));
}

FileIoContext::FileIoContext(const base::string16& file_name,
                             const base::string16& mode,
                             const domapi::OpenFileDeferred& deferred)
    : file_handle_(OpenFile(file_name, mode, deferred)), operation_(nullptr) {
  TRACE_EVENT_ASYNC_BEGIN0("io", "FileContext", this);
  if (!file_handle_.is_valid())
    return;
  overlapped = {0};
  handler = this;
  editor::Application::instance()->io_manager()->RegisterIoHandler(
      file_handle_.get(), this);
}

FileIoContext::~FileIoContext() {
  TRACE_EVENT_ASYNC_END0("io", "FileContext", this);
}

// base::MessagePumpForIO::FileIoContext
void FileIoContext::OnIOCompleted(IOContext*,
                                  DWORD bytes_transferred,
                                  DWORD error) {
  DCHECK(IsRunning());
  TRACE_EVENT_WITH_FLOW1("script", "FileIoContext::OnIOCompleted",
                         deferred_.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "operation", operation_);
  overlapped.Offset += bytes_transferred;
  auto const operation = operation_;
  operation_ = nullptr;
  if (!error || error == ERROR_HANDLE_EOF)
    Resolve(deferred_.resolve, bytes_transferred);
  else
    Reject(deferred_.reject, error);
  if (!file_handle_.is_valid())
    delete this;
  TRACE_EVENT_ASYNC_END0("io", operation, deferred_.sequence_num);
}

// io::IoContext
void FileIoContext::Close(const domapi::FileIoDeferred& deferred) {
  TRACE_EVENT_WITH_FLOW0("script", "FileIoContext::Close",
                         deferred.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT);
  if (file_handle_.is_valid()) {
    if (!::CloseHandle(file_handle_.get())) {
      auto const last_error = ::GetLastError();
      DVLOG(0) << "CloseHandle error=" << last_error;
      Reject(deferred.reject, last_error);
      return;
    }
    file_handle_.release();
  }
  if (!IsRunning())
    delete this;
  Resolve(deferred.resolve, 0u);
}

void FileIoContext::Read(void* buffer,
                         size_t num_read,
                         const domapi::FileIoDeferred& deferred) {
  TRACE_EVENT_WITH_FLOW0("script", "FileIoContext::Read", deferred.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT);
  if (IsRunning()) {
    Reject(deferred.reject, ERROR_BUSY);
    return;
  }

  deferred_ = deferred;
  operation_ = "ReadFile";
  TRACE_EVENT_ASYNC_BEGIN0("io", operation_, deferred.sequence_num);
  DWORD read;
  auto const succeeded =
      ::ReadFile(file_handle_.get(), buffer, static_cast<DWORD>(num_read),
                 &read, &overlapped);
  DCHECK(!succeeded);
  __assume(!succeeded);

  auto const error = ::GetLastError();
  if (error == ERROR_IO_PENDING)
    return;
  OnIOCompleted(this, 0, error);
}

void FileIoContext::Write(void* buffer,
                          size_t num_write,
                          const domapi::FileIoDeferred& deferred) {
  TRACE_EVENT_WITH_FLOW0("script", "FileIoContext::Write",
                         deferred.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT);
  if (IsRunning()) {
    Reject(deferred.reject, ERROR_BUSY);
    return;
  }

  deferred_ = deferred;
  operation_ = "WriteFile";
  TRACE_EVENT_ASYNC_BEGIN0("io", operation_, deferred.sequence_num);
  DWORD written;
  auto const succeeded =
      ::WriteFile(file_handle_.get(), buffer, static_cast<DWORD>(num_write),
                  &written, &overlapped);
  DCHECK(!succeeded);
  __assume(!succeeded);
  auto const error = ::GetLastError();
  if (error == ERROR_IO_PENDING)
    return;
  OnIOCompleted(this, 0, error);
}

}  // namespace io
