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

void ProcessIoContext::CloseProcess() {
  if (read_pipe_.is_valid())
    ::CloseHandle(read_pipe_.release());
  if (write_pipe_.is_valid())
    ::CloseHandle(write_pipe_.release());
  auto const value = ::WaitForSingleObject(process_.get(), INFINITE);
  if (value == WAIT_FAILED) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "WaitSingleObject", last_error);
  }
}

void ProcessIoContext::ReadFromProcess(
    void* buffer, size_t num_read, const domapi::FileIoCallback& callback) {
  DWORD read;
  auto const succeeded = ::ReadFile(read_pipe_.get(), buffer, num_read, &read,
                                    nullptr);
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
  if (!::CreatePipe(read_pipe_.location(), write_pipe_.location(),
                    nullptr, 0)) {
    auto const last_error = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "CreatePipe", last_error);
    RunCallback(callback, last_error);
    return;
  }

  PROCESS_INFORMATION process_info;
  STARTUPINFO startup_info = {0};
  startup_info.cb = sizeof(startup_info);
  startup_info.dwFlags = STARTF_USESTDHANDLES;
  startup_info.hStdInput = write_pipe_.get();
  startup_info.hStdOutput = read_pipe_.get();
  startup_info.hStdError = read_pipe_.get();
  std::vector<base::char16> command_line_string(command_line.length() + 1);
  ::memcpy(command_line_string.data(), command_line.data(),
           command_line.length() * sizeof(base::char16));
  command_line_string[command_line.length() + 1] = 0;
  auto const succeeded = ::CreateProcessW(
      nullptr,
      &command_line_string[0],
      nullptr,
      nullptr,
      false,
      CREATE_SUSPENDED,
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
}

void ProcessIoContext::WriteToProcess(
    void* buffer, size_t num_write, const domapi::FileIoCallback& callback) {
  DWORD written;
  auto const succeeded = ::WriteFile(write_pipe_.get(), buffer, num_write,
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
void ProcessIoContext::Close() {
  gateway_thread_->message_loop()->PostTask(FROM_HERE, base::Bind(
      &ProcessIoContext::CloseProcess, base::Unretained(this)));
}

void ProcessIoContext::Read(void* buffer, size_t num_read,
                            const domapi::FileIoCallback& callback) {
  gateway_thread_->message_loop()->PostTask(FROM_HERE, base::Bind(
      &ProcessIoContext::ReadFromProcess, base::Unretained(this),
      base::Unretained(buffer), num_read, callback));
}

void ProcessIoContext::Write(void* buffer, size_t num_write,
                             const domapi::FileIoCallback& callback) {
  gateway_thread_->message_loop()->PostTask(FROM_HERE, base::Bind(
      &ProcessIoContext::WriteToProcess, base::Unretained(this),
      base::Unretained(buffer), num_write, callback));
}

}  // namespace io
