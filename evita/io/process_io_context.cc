// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/process_io_context.h"

#include "base/bind.h"
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/threading/thread.h"
#pragma warning(pop)
#include "evita/dom/public/deferred.h"
#include "evita/dom/public/io_error.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/io/io_context_utils.h"

#define DVLOG_WIN32_ERROR(level, name, last_error) \
  DVLOG(level) << name ": " << this << " err=" << last_error

namespace io {

void Resolve(const base::Callback<void(domapi::ProcessId)>& resolve,
             domapi::ProcessId context_id) {
  editor::Application::instance()->view_event_handler()->RunCallback(
      base::Bind(resolve , context_id));
}

ProcessIoContext::ProcessIoContext(domapi::IoContextId context_id,
                 const base::string16& command_line,
                 const domapi::OpenProcessDeferred& deferred)
    : gateway_thread_(new base::Thread("process")) {
  gateway_thread_->Start();
  gateway_thread_->message_loop()->PostTask(FROM_HERE,
    base::Bind(&ProcessIoContext::StartProcess,
               base::Unretained(this), context_id, command_line, deferred));
}

ProcessIoContext::~ProcessIoContext() {
}

void ProcessIoContext::CloseAndWaitProcess(
    const domapi::FileIoDeferred& deferred) {
  if (auto const last_error = CloseProcess()) {
    Reject(deferred.reject, last_error);
    return;
  }

  if (process_.is_valid()) {
    auto const value = ::WaitForSingleObject(process_.get(), INFINITE);
    if (value == WAIT_FAILED) {
      auto const last_error = ::GetLastError();
      DVLOG_WIN32_ERROR(0, "WaitSingleObject", last_error);
      Reject(deferred.reject, last_error);
      return;
    }
    process_.release();
  }
  Resolve(deferred.resolve, 0u);
}

DWORD ProcessIoContext::CloseProcess() {
  if (stdout_read_.is_valid()) {
    if (!::CloseHandle(stdout_read_.get())) {
        auto const last_error = ::GetLastError();
        DVLOG_WIN32_ERROR(0, "CloseHandle", last_error);
        return last_error;
    }
    stdout_read_.release();
  }
  if (stdin_write_.is_valid()) {
    if (!::CloseHandle(stdin_write_.get())) {
        auto const last_error = ::GetLastError();
        DVLOG_WIN32_ERROR(0, "CloseHandle", last_error);
        return last_error;
    }
    stdin_write_.release();
  }
  if (gateway_thread_->IsRunning())
    gateway_thread_->message_loop()->QuitWhenIdle();
  return 0;
}

void ProcessIoContext::ReadFromProcess(
    void* buffer, size_t num_read, const domapi::FileIoDeferred& deferred) {
  DWORD read;
  auto const succeeded = ::ReadFile(stdout_read_.get(), buffer, num_read,
                                    &read, nullptr);
  if (!succeeded) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "ReadFile", last_error);
    if (last_error == ERROR_BROKEN_PIPE)
      CloseProcess();
    Reject(deferred.reject, last_error);
    return;
  }
  Resolve(deferred.resolve, read);
}

void ProcessIoContext::StartProcess(domapi::IoContextId context_id,
    const base::string16& command_line,
    const domapi::OpenProcessDeferred& deferred) {
  SECURITY_ATTRIBUTES security_attributes = {0};
  security_attributes.nLength = sizeof(security_attributes);
  security_attributes.bInheritHandle = true;
  common::win::scoped_handle stdin_read;
  common::win::scoped_handle stdin_write;
  if (!::CreatePipe(stdin_read.location(), stdin_write.location(),
                    &security_attributes, 0)) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "CreatePipe", last_error);
    Reject(deferred.reject, last_error);
    return;
  }
  if (!::SetHandleInformation(stdin_write.get(), HANDLE_FLAG_INHERIT, 0)) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "SetHandleInformation", last_error);
    Reject(deferred.reject, last_error);
    return;
  }
  common::win::scoped_handle stdout_read;
  common::win::scoped_handle stdout_write;
  if (!::CreatePipe(stdout_read.location(), stdout_write.location(),
                    &security_attributes, 0)) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "CreatePipe", last_error);
    Reject(deferred.reject, last_error);
    return;
  }
  if (!::SetHandleInformation(stdout_read.get(), HANDLE_FLAG_INHERIT, 0)) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "SetHandleInformation", last_error);
    Reject(deferred.reject, last_error);
    return;
  }

  PROCESS_INFORMATION process_info;
  STARTUPINFO startup_info = {0};
  startup_info.cb = sizeof(startup_info);
  startup_info.dwFlags = STARTF_USESTDHANDLES;
  startup_info.hStdInput = stdin_read.get();
  startup_info.hStdOutput = stdout_write.get();
  startup_info.hStdError = stdout_write.get();
  std::vector<base::char16> command_line_string(command_line.length() + 1);
  ::memcpy(command_line_string.data(), command_line.data(),
           command_line.length() * sizeof(base::char16));
  command_line_string[command_line.length()] = 0;
  auto const inherit_handles = true;
  auto const succeeded = ::CreateProcessW(
      nullptr,
      &command_line_string[0],
      nullptr,
      nullptr,
      inherit_handles,
      CREATE_NO_WINDOW | CREATE_SUSPENDED,
      nullptr,
      nullptr,
      &startup_info,
      &process_info);
  if (!succeeded) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "CreateProcessW", last_error);
    Reject(deferred.reject, last_error);
    return;
  }
  ::ResumeThread(process_info.hThread);
  process_.reset(process_info.hProcess);
  ::CloseHandle(process_info.hThread);
  Resolve(deferred.resolve, domapi::ProcessId(context_id));
  stdin_read.release();
  stdin_write_.reset(stdin_write.release());
  stdout_write.release();
  stdout_read_.reset(stdout_read.release());
}

void ProcessIoContext::WriteToProcess(
    void* buffer, size_t num_write, const domapi::FileIoDeferred& deferred) {
  DWORD written;
  auto const succeeded = ::WriteFile(stdin_write_.get(), buffer, num_write,
                                     &written, nullptr);
  if (!succeeded) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "WriteFile", last_error);
    if (last_error == ERROR_BROKEN_PIPE)
      CloseProcess();
    Reject(deferred.reject, last_error);
    return;
  }
  Resolve(deferred.resolve, written);
}

// io::IoContext
void ProcessIoContext::Close(const domapi::FileIoDeferred& deferred) {
  if (!gateway_thread_->IsRunning()) {
    Reject(deferred.reject, ERROR_INVALID_HANDLE);
    return;
  }
  gateway_thread_->message_loop()->PostTask(FROM_HERE, base::Bind(
      &ProcessIoContext::CloseAndWaitProcess,
      base::Unretained(this), deferred));
}

void ProcessIoContext::Read(void* buffer, size_t num_read,
                            const domapi::FileIoDeferred& deferred) {
  if (!gateway_thread_->IsRunning() || !stdout_read_.is_valid()) {
    Reject(deferred.reject, ERROR_INVALID_HANDLE);
    return;
  }
  gateway_thread_->message_loop()->PostTask(FROM_HERE, base::Bind(
      &ProcessIoContext::ReadFromProcess, base::Unretained(this),
      base::Unretained(buffer), num_read, deferred));
}

void ProcessIoContext::Write(void* buffer, size_t num_write,
                             const domapi::FileIoDeferred& deferred) {
  if (!gateway_thread_->IsRunning() || !stdin_write_.is_valid()) {
    Reject(deferred.reject, ERROR_INVALID_HANDLE);
    return;
  }

  // Abort blocked read pipe operation.
  // We'll get ERROR_OPERATION_ABORTED(995).
  auto const hThread = reinterpret_cast<HANDLE>(
      gateway_thread_->thread_handle().platform_handle());
  if (!::CancelSynchronousIo(hThread)) {
    auto const last_error = ::GetLastError();
    if (last_error != ERROR_NOT_FOUND) {
      DVLOG_WIN32_ERROR(0, "CancelSynchronousIo", last_error);
      Reject(deferred.reject, last_error);
      return;
    }
  }

  gateway_thread_->message_loop()->PostTask(FROM_HERE, base::Bind(
      &ProcessIoContext::WriteToProcess, base::Unretained(this),
      base::Unretained(buffer), num_write, deferred));
}

}  // namespace io
