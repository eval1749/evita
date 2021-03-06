// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/process_io_context.h"

#include <windows.h>

#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/public/io_error.h"
#include "evita/dom/public/promise.h"
#include "evita/io/io_context_utils.h"

namespace io {

ProcessIoContext::ProcessIoContext(domapi::IoContextId context_id,
                                   const base::string16& command_line,
                                   domapi::OpenProcessPromise promise)
    : gateway_thread_(new base::Thread("process")) {
  TRACE_EVENT_ASYNC_BEGIN1("io", "ProcessContext", this, "command_line",
                           base::UTF16ToUTF8(command_line).c_str());
  gateway_thread_->Start();
  gateway_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&ProcessIoContext::StartProcess, base::Unretained(this),
                     context_id, command_line, std::move(promise)));
}

ProcessIoContext::~ProcessIoContext() {
  TRACE_EVENT_ASYNC_END0("io", "ProcessContext", this);
}

void ProcessIoContext::CloseAndWaitProcess(domapi::IoIntPromise promise) {
  TRACE_EVENT0("io", "ProcessIoContext::CloseAndWaitProcess");
  TRACE_EVENT_WITH_FLOW1("promise", "Promise", promise.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "ProcessIoContext::CloseAndWaitProcess");
  if (auto const last_error = CloseProcess()) {
    Reject(std::move(promise.reject), last_error);
    return;
  }

  if (process_.is_valid()) {
    auto const value = ::WaitForSingleObject(process_.get(), INFINITE);
    if (value == WAIT_FAILED) {
      const auto last_error = ::GetLastError();
      PLOG(ERROR) << "WaitSingleObject failed";
      Reject(std::move(promise.reject), last_error);
      return;
    }
    process_.release();
  }
  Resolve(std::move(promise.resolve), 0u);
}

uint32_t ProcessIoContext::CloseProcess() {
  TRACE_EVENT0("io", "ProcessIoContext::CloseProcess");
  if (stdout_read_.is_valid()) {
    if (!::CloseHandle(stdout_read_.get())) {
      const auto last_error = ::GetLastError();
      PLOG(ERROR) << "CloseHandle failed";
      return last_error;
    }
    stdout_read_.release();
  }
  if (stdin_write_.is_valid()) {
    if (!::CloseHandle(stdin_write_.get())) {
      const auto last_error = ::GetLastError();
      PLOG(ERROR) << "CloseHandle failed";
      return last_error;
    }
    stdin_write_.release();
  }
  if (gateway_thread_->IsRunning())
    gateway_thread_->message_loop()->task_runner()->PostTask(
        FROM_HERE, base::MessageLoop::QuitWhenIdleClosure());
  return 0;
}

void ProcessIoContext::ReadFromProcess(void* buffer,
                                       size_t num_read,
                                       domapi::IoIntPromise promise) {
  TRACE_EVENT_WITH_FLOW1("promise", "ProcessIoContext::ReadFromProcess",
                         promise.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "ProcessIoContext::ReadFromProcess");
  DWORD read;
  auto const succeeded = ::ReadFile(
      stdout_read_.get(), buffer, static_cast<DWORD>(num_read), &read, nullptr);
  if (!succeeded) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "ReadFile failed";
    if (last_error == ERROR_BROKEN_PIPE)
      CloseProcess();
    Reject(std::move(promise.reject), last_error);
    return;
  }
  Resolve(std::move(promise.resolve), read);
}

void ProcessIoContext::StartProcess(domapi::IoContextId context_id,
                                    const base::string16& command_line,
                                    domapi::OpenProcessPromise promise) {
  TRACE_EVENT_WITH_FLOW1("promise", "ProcessIoContext::StartProcess",
                         promise.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "ProcessIoContext::StartProcess");
  SECURITY_ATTRIBUTES security_attributes = {0};
  security_attributes.nLength = sizeof(security_attributes);
  security_attributes.bInheritHandle = true;
  common::win::scoped_handle stdin_read;
  common::win::scoped_handle stdin_write;
  if (!::CreatePipe(stdin_read.location(), stdin_write.location(),
                    &security_attributes, 0)) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "CreatePipe failed";
    Reject(std::move(promise.reject), last_error);
    return;
  }
  if (!::SetHandleInformation(stdin_write.get(), HANDLE_FLAG_INHERIT, 0)) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "SetHandleInformation failed";
    Reject(std::move(promise.reject), last_error);
    return;
  }
  common::win::scoped_handle stdout_read;
  common::win::scoped_handle stdout_write;
  if (!::CreatePipe(stdout_read.location(), stdout_write.location(),
                    &security_attributes, 0)) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "CreatePipe failed";
    Reject(std::move(promise.reject), last_error);
    return;
  }
  if (!::SetHandleInformation(stdout_read.get(), HANDLE_FLAG_INHERIT, 0)) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "SetHandleInformation failed";
    Reject(std::move(promise.reject), last_error);
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
  auto const succeeded =
      ::CreateProcessW(nullptr, &command_line_string[0], nullptr, nullptr,
                       inherit_handles, CREATE_NO_WINDOW | CREATE_SUSPENDED,
                       nullptr, nullptr, &startup_info, &process_info);
  if (!succeeded) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "CreateProcessW failed";
    Reject(std::move(promise.reject), last_error);
    return;
  }
  ::ResumeThread(process_info.hThread);
  process_.reset(process_info.hProcess);
  ::CloseHandle(process_info.hThread);
  RunCallback(base::BindOnce(std::move(promise.resolve),
                             domapi::ProcessId(context_id)));
  stdin_read.release();
  stdin_write_.reset(stdin_write.release());
  stdout_write.release();
  stdout_read_.reset(stdout_read.release());
}

void ProcessIoContext::WriteToProcess(void* buffer,
                                      size_t num_write,
                                      domapi::IoIntPromise promise) {
  TRACE_EVENT_WITH_FLOW1("promise", "ProcessIoContext::WriteToProcess",
                         promise.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "ProcessIoContext::WriteToProcess");
  DWORD written;
  auto const succeeded =
      ::WriteFile(stdin_write_.get(), buffer, static_cast<DWORD>(num_write),
                  &written, nullptr);
  if (!succeeded) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "WriteFile failed";
    if (last_error == ERROR_BROKEN_PIPE)
      CloseProcess();
    Reject(std::move(promise.reject), last_error);
    return;
  }
  Resolve(std::move(promise.resolve), written);
}

// io::IoContext
void ProcessIoContext::Close(domapi::IoIntPromise promise) {
  TRACE_EVENT0("io", "ProcessIoContext::Close");
  if (!gateway_thread_->IsRunning()) {
    Reject(std::move(promise.reject), ERROR_INVALID_HANDLE);
    return;
  }
  gateway_thread_->task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&ProcessIoContext::CloseAndWaitProcess,
                                base::Unretained(this), std::move(promise)));
}

void ProcessIoContext::Read(void* buffer,
                            size_t num_read,
                            domapi::IoIntPromise promise) {
  if (!gateway_thread_->IsRunning() || !stdout_read_.is_valid()) {
    Reject(std::move(promise.reject), ERROR_INVALID_HANDLE);
    return;
  }
  gateway_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&ProcessIoContext::ReadFromProcess, base::Unretained(this),
                     base::Unretained(buffer), num_read, std::move(promise)));
}

void ProcessIoContext::Write(void* buffer,
                             size_t num_write,
                             domapi::IoIntPromise promise) {
  if (!gateway_thread_->IsRunning() || !stdin_write_.is_valid()) {
    Reject(std::move(promise.reject), ERROR_INVALID_HANDLE);
    return;
  }

#if 0
  // TODO(eval1749): Since http://crrev.com/406831,
  // |base::Thread::thread_handle()| is no long avaialble, we should find the
  // way to have thread handle.
  // Abort blocked read pipe operation.
  // We'll get ERROR_OPERATION_ABORTED(995).
  auto const hThread = reinterpret_cast<HANDLE>(
      gateway_thread_->thread_handle().platform_handle());
  if (!::CancelSynchronousIo(hThread)) {
    const auto last_error = ::GetLastError();
    if (last_error != ERROR_NOT_FOUND) {
      PLOG(ERROR) << "CancelSynchronousIo failed";
      Reject(std::move(promise.reject), last_error);
      return;
    }
  }
#endif

  gateway_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&ProcessIoContext::WriteToProcess, base::Unretained(this),
                     base::Unretained(buffer), num_write, std::move(promise)));
}

}  // namespace io
