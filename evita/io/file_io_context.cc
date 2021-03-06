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

}  // namespace

void Resolve(base::OnceCallback<void(domapi::FileId)> resolve,
             domapi::FileId context_id) {
  RunCallback(base::BindOnce(std::move(resolve), context_id));
}

FileIoContext::FileIoContext(HANDLE handle) : file_handle_(handle) {
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
  TRACE_EVENT_WITH_FLOW1("promise", "FileIoContext::OnIOCompleted",
                         promise_.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "operation", operation_);
  overlapped.Offset += bytes_transferred;
  auto const operation = operation_;
  operation_ = nullptr;
  if (!error || error == ERROR_HANDLE_EOF)
    Resolve(std::move(promise_.resolve), bytes_transferred);
  else
    Reject(std::move(promise_.reject), error);
  if (!file_handle_.is_valid())
    delete this;
  TRACE_EVENT_ASYNC_END0("io", operation, promise_.sequence_num);
}

// io::IoContext
void FileIoContext::Close(domapi::IoIntPromise promise) {
  TRACE_EVENT_WITH_FLOW0("promise", "FileIoContext::Close",
                         promise.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT);
  if (file_handle_.is_valid()) {
    if (!::CloseHandle(file_handle_.get())) {
      const auto last_error = ::GetLastError();
      PLOG(ERROR) << "CloseHandle failed.";
      Reject(std::move(promise.reject), last_error);
      return;
    }
    file_handle_.release();
  }
  if (!IsRunning())
    delete this;
  Resolve(std::move(promise.resolve), 0u);
}

// static
std::pair<HANDLE, int> FileIoContext::Open(const base::string16& file_name,
                                           const base::string16& mode) {
  CreateFileParams params(mode);
  common::win::scoped_handle handle(
      ::CreateFileW(file_name.c_str(), params.access, params.share_mode,
                    nullptr, params.creation, FILE_FLAG_OVERLAPPED, nullptr));
  if (!handle) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "CreateFileW " << file_name << " failed.";
    return std::make_pair(INVALID_HANDLE_VALUE, last_error);
  }
  return std::make_pair(handle.release(), 0);
}

void FileIoContext::Read(void* buffer,
                         size_t num_read,
                         domapi::IoIntPromise promise) {
  TRACE_EVENT_WITH_FLOW0("promise", "FileIoContext::Read", promise.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT);
  if (IsRunning()) {
    Reject(std::move(promise.reject), ERROR_BUSY);
    return;
  }

  promise_ = std::move(promise);
  operation_ = "ReadFile";
  TRACE_EVENT_ASYNC_BEGIN0("io", operation_, promise.sequence_num);
  DWORD read;
  auto const succeeded =
      ::ReadFile(file_handle_.get(), buffer, static_cast<DWORD>(num_read),
                 &read, &overlapped);
  DCHECK(!succeeded);
  __assume(!succeeded);

  auto const error = ::GetLastError();
  if (error == ERROR_IO_PENDING)
    return;
  PLOG(ERROR) << "ReadFile failed";
  OnIOCompleted(this, 0, error);
}

void FileIoContext::Write(void* buffer,
                          size_t num_write,
                          domapi::IoIntPromise promise) {
  TRACE_EVENT_WITH_FLOW0("promise", "FileIoContext::Write",
                         promise.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT);
  if (IsRunning()) {
    Reject(std::move(promise.reject), ERROR_BUSY);
    return;
  }

  promise_ = std::move(promise);
  operation_ = "WriteFile";
  TRACE_EVENT_ASYNC_BEGIN0("io", operation_, promise.sequence_num);
  DWORD written;
  auto const succeeded =
      ::WriteFile(file_handle_.get(), buffer, static_cast<DWORD>(num_write),
                  &written, &overlapped);
  DCHECK(!succeeded);
  __assume(!succeeded);
  auto const error = ::GetLastError();
  if (error == ERROR_IO_PENDING)
    return;
  PLOG(ERROR) << "WriteFile failed";
  OnIOCompleted(this, 0, error);
}

}  // namespace io
