// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/process_io_context.h"

#include "base/bind.h"
//#include "base/callback.h"
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/threading/thread.h"
#pragma warning(pop)
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"

#define DVLOG_WIN32_ERROR(level, name, last_error) \
  DVLOG(level) << name ": " << this << " err=" << last_error

namespace io {

namespace {
void RunCallback(const domapi::FileIoCallback& callback, DWORD num_transferred,
                 DWORD last_error) {
    Application::instance()->view_event_handler()->RunCallback(
        base::Bind(callback, static_cast<int>(num_transferred),
                   static_cast<int>(last_error)));
}

void RunCallback(const domapi::CloseFileCallback& callback, DWORD last_error) {
    Application::instance()->view_event_handler()->RunCallback(
        base::Bind(callback, static_cast<int>(last_error)));
}

void RunCallback(const domapi::NewProcessCallback& callback,
                 DWORD last_error) {
    Application::instance()->view_event_handler()->RunCallback(
        base::Bind(callback, domapi::IoContextId(),
                   static_cast<int>(last_error)));
}
}  // namespace

ProcessIoContext::ProcessIoContext(domapi::IoContextId context_id,
                 const base::string16& command_line,
                 const domapi::NewProcessCallback& callback)
    : gateway_thread_(new base::Thread("process")) {
  gateway_thread_->Start();
  gateway_thread_->message_loop()->PostTask(FROM_HERE,
    base::Bind(&ProcessIoContext::StartProcess,
               base::Unretained(this), context_id, command_line, callback));
}

ProcessIoContext::~ProcessIoContext() {
}

void ProcessIoContext::CloseProcess(
    const domapi::CloseFileCallback& callback) {
  if (stdout_read_.is_valid()) {
    if (!::CloseHandle(stdout_read_.get())) {
        auto const last_error = ::GetLastError();
        DVLOG_WIN32_ERROR(0, "CloseHandle", last_error);
        RunCallback(callback, last_error);
        return;
    }
    stdout_read_.release();
  }
  if (stdin_write_.is_valid()) {
    if (!::CloseHandle(stdin_write_.get())) {
        auto const last_error = ::GetLastError();
        DVLOG_WIN32_ERROR(0, "CloseHandle", last_error);
        RunCallback(callback, last_error);
        return;
    }
    stdin_write_.release();
  }
  gateway_thread_->message_loop()->QuitWhenIdle();
  auto const value = ::WaitForSingleObject(process_.get(), INFINITE);
  if (value == WAIT_FAILED) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "WaitSingleObject", last_error);
    RunCallback(callback, last_error);
    return;
  }
  process_.release();
  RunCallback(callback, 0);
}

void ProcessIoContext::ReadFromProcess(
    void* buffer, size_t num_read, const domapi::FileIoCallback& callback) {
  DWORD read;
  auto const succeeded = ::ReadFile(stdout_read_.get(), buffer, num_read,
                                    &read, nullptr);
  if (!succeeded) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "ReadFile", last_error);
    RunCallback(callback, 0, last_error);
    return;
  }
  RunCallback(callback, read, 0);
}

void ProcessIoContext::StartProcess(domapi::IoContextId context_id,
    const base::string16& command_line,
    const domapi::NewProcessCallback& callback) {
  SECURITY_ATTRIBUTES security_attributes = {0};
  security_attributes.nLength = sizeof(security_attributes);
  security_attributes.bInheritHandle = true;
  common::win::scoped_handle stdin_read;
  common::win::scoped_handle stdin_write;
  if (!::CreatePipe(stdin_read.location(), stdin_write.location(),
                    &security_attributes, 0)) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "CreatePipe", last_error);
    RunCallback(callback, last_error);
    return;
  }
  if (!::SetHandleInformation(stdin_write.get(), HANDLE_FLAG_INHERIT, 0)) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "SetHandleInformation", last_error);
    RunCallback(callback, last_error);
    return;
  }
  common::win::scoped_handle stdout_read;
  common::win::scoped_handle stdout_write;
  if (!::CreatePipe(stdout_read.location(), stdout_write.location(),
                    &security_attributes, 0)) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "CreatePipe", last_error);
    RunCallback(callback, last_error);
    return;
  }
  if (!::SetHandleInformation(stdout_read.get(), HANDLE_FLAG_INHERIT, 0)) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "SetHandleInformation", last_error);
    RunCallback(callback, last_error);
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
    RunCallback(callback, last_error);
    return;
  }
  ::ResumeThread(process_info.hThread);
  process_.reset(process_info.hProcess);
  ::CloseHandle(process_info.hThread);
  Application::instance()->view_event_handler()->RunCallback(
    base::Bind(callback, context_id, 0));
  stdin_read.release();
  stdin_write_.reset(stdin_write.release());
  stdout_write.release();
  stdout_read_.reset(stdout_read.release());
}

void ProcessIoContext::WriteToProcess(
    void* buffer, size_t num_write, const domapi::FileIoCallback& callback) {
  DWORD written;
  auto const succeeded = ::WriteFile(stdin_write_.get(), buffer, num_write,
                                     &written, nullptr);
  if (!succeeded) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "WriteFile", last_error);
    RunCallback(callback, 0, last_error);
    return;
  }
  RunCallback(callback, written, 0);
}

// io::IoContext
void ProcessIoContext::Close(const domapi::CloseFileCallback& callback) {
  if (!gateway_thread_->IsRunning()) {
    RunCallback(callback, ERROR_INVALID_HANDLE);
    return;
  }
  gateway_thread_->message_loop()->PostTask(FROM_HERE, base::Bind(
      &ProcessIoContext::CloseProcess, base::Unretained(this), callback));
}

void ProcessIoContext::Read(void* buffer, size_t num_read,
                            const domapi::FileIoCallback& callback) {
  if (!gateway_thread_->IsRunning() || !stdout_read_.is_valid()) {
    RunCallback(callback, 0, ERROR_INVALID_HANDLE);
    return;
  }
  gateway_thread_->message_loop()->PostTask(FROM_HERE, base::Bind(
      &ProcessIoContext::ReadFromProcess, base::Unretained(this),
      base::Unretained(buffer), num_read, callback));
}

void ProcessIoContext::Write(void* buffer, size_t num_write,
                             const domapi::FileIoCallback& callback) {
  if (!gateway_thread_->IsRunning() || !stdin_write_.is_valid()) {
    RunCallback(callback, 0, ERROR_INVALID_HANDLE);
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
      RunCallback(callback, 0, last_error);
      return;
    }
  }

  gateway_thread_->message_loop()->PostTask(FROM_HERE, base::Bind(
      &ProcessIoContext::WriteToProcess, base::Unretained(this),
      base::Unretained(buffer), num_write, callback));
}

}  // namespace io
