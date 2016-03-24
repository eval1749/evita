// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/io_delegate_impl.h"

#include <unordered_map>
#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_pump_win.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"
#include "common/win/scoped_handle.h"
#include "evita/dom/public/io_context_id.h"
#include "evita/dom/public/io_error.h"
#include "evita/dom/public/promise.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/io/directory_io_context.h"
#include "evita/io/file_io_context.h"
#include "evita/io/io_context_utils.h"
#include "evita/io/process_io_context.h"
#include "evita/io/win_resource_io_context.h"
#include "evita/spellchecker/spelling_engine.h"

#define DVLOG_WIN32_ERROR(level, name) \
  DVLOG(level) << name ": " << this << " " << file_name << " err=" << dwError

namespace io {

namespace {
const uint32_t kHugeFileSize = 1u << 28;

class scoped_find_handle final {
 public:
  explicit scoped_find_handle(HANDLE handle) : handle_(handle) {}

  ~scoped_find_handle() {
    if (is_valid())
      ::FindClose(handle_);
  }

  bool is_valid() const { return handle_ != INVALID_HANDLE_VALUE; }

 private:
  HANDLE handle_;

  DISALLOW_COPY_AND_ASSIGN(scoped_find_handle);
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// IoDelegateImpl
//
IoDelegateImpl::IoDelegateImpl() {}

IoDelegateImpl::~IoDelegateImpl() {}

IoContext* IoDelegateImpl::IoContextOf(domapi::IoContextId context_id) const {
  const auto& it = context_map_.find(context_id);
  if (it == context_map_.end())
    return nullptr;
  return it->second;
}

// domapi::IoDelegate
void IoDelegateImpl::CheckSpelling(const base::string16& word_to_check,
                                   const CheckSpellingResolver& promise) {
  TRACE_EVENT_WITH_FLOW1("promise", "Promise", promise.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::CheckSpelling");
  RunCallback(base::Bind(
      promise.resolve,
      spellchecker::SpellingEngine::GetSpellingEngine()->CheckSpelling(
          word_to_check)));
}

void IoDelegateImpl::CloseContext(const domapi::IoContextId& context_id,
                                  const domapi::IoIntPromise& promise) {
  const auto& it = context_map_.find(context_id);
  if (it == context_map_.end())
    return Reject(promise.reject, ERROR_INVALID_HANDLE);
  const auto context = it->second->as<IoContext>();
  if (!context)
    return Reject(promise.reject, ERROR_INVALID_HANDLE);
  context->Close(promise);
  context_map_.erase(it);
}

void IoDelegateImpl::GetWinResourceNames(
    const domapi::WinResourceId& resource_id,
    const base::string16& type,
    const GetWinResourceNamessPromise& promise) {
  const auto& it = context_map_.find(resource_id);
  if (it == context_map_.end())
    return Reject(promise.reject, ERROR_INVALID_HANDLE);
  const auto resource = it->second->as<WinResourceIoContext>();
  if (!resource)
    return Reject(promise.reject, ERROR_INVALID_HANDLE);
  const auto& pair = resource->GetResourceName(type);
  if (pair.second)
    return Reject(promise.reject, pair.second);
  return RunCallback(base::Bind(promise.resolve, pair.first));
}

void IoDelegateImpl::GetSpellingSuggestions(
    const base::string16& wrong_word,
    const GetSpellingSuggestionsResolver& promise) {
  TRACE_EVENT_WITH_FLOW1("promise", "Promise", promise.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::GetSpellingSuggestions");
  RunCallback(base::Bind(
      promise.resolve,
      spellchecker::SpellingEngine::GetSpellingEngine()->GetSpellingSuggestions(
          wrong_word)));
}

void IoDelegateImpl::LoadWinResource(const domapi::WinResourceId& resource_id,
                                     const base::string16& type,
                                     const base::string16& name,
                                     uint8_t* buffer,
                                     size_t buffer_size,
                                     const domapi::IoIntPromise& promise) {
  const auto& it = context_map_.find(resource_id);
  if (it == context_map_.end())
    return Reject(promise.reject, ERROR_INVALID_HANDLE);
  const auto resource = it->second->as<WinResourceIoContext>();
  if (!resource)
    return Reject(promise.reject, ERROR_INVALID_HANDLE);
  const auto& pair = resource->Load(type, name, buffer, buffer_size);
  if (pair.second)
    return Reject(promise.reject, pair.second);
  return RunCallback(base::Bind(promise.resolve, pair.first));
}

void IoDelegateImpl::MakeTempFileName(
    const base::string16& dir_name,
    const base::string16& prefix,
    const domapi::MakeTempFileNamePromise& resolver) {
  TRACE_EVENT_WITH_FLOW1("promise", "Promise", resolver.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::MakeTempFileName");
  base::string16 file_name(MAX_PATH, 0);
  const auto unique_id =
      ::GetTempFileNameW(dir_name.c_str(), prefix.c_str(), 0, &file_name[0]);
  if (!unique_id) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "GetTempFileName failed";
    Reject(resolver.reject, last_error);
    return;
  }
  const auto nul_pos = file_name.find(static_cast<base::char16>(0));
  if (nul_pos != base::string16::npos)
    file_name.resize(nul_pos);
  RunCallback(base::Bind(resolver.resolve, file_name));
}

void IoDelegateImpl::MoveFile(const base::string16& src_path,
                              const base::string16& dst_path,
                              const domapi::MoveFileOptions& options,
                              const domapi::IoBoolPromise& resolver) {
  TRACE_EVENT0("io", "IoDelegateImpl::MoveFile");
  TRACE_EVENT_WITH_FLOW1("promise", "Promise", resolver.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::MoveFile");
  const auto flags = options.no_overwrite
                         ? MOVEFILE_WRITE_THROUGH
                         : MOVEFILE_WRITE_THROUGH | MOVEFILE_REPLACE_EXISTING;
  const auto succeeded = ::MoveFileExW(src_path.c_str(), dst_path.c_str(),
                                       static_cast<uint32_t>(flags));
  if (!succeeded) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "MoveFileEx failed";
    Reject(resolver.reject, last_error);
    return;
  }
  RunCallback(base::Bind(resolver.resolve, true));
}

void IoDelegateImpl::OpenDirectory(
    const base::string16& dir_name,
    const domapi::OpenDirectoryPromise& promise) {
  auto directory = std::make_unique<DirectoryIoContext>(dir_name);
  const auto directory_id = domapi::IoContextId::New();
  context_map_.insert(std::make_pair(directory_id, directory.release()));
  RunCallback(base::Bind(promise.resolve, domapi::DirectoryId(directory_id)));
}

void IoDelegateImpl::OpenFile(const base::string16& file_name,
                              const base::string16& mode,
                              const domapi::OpenFilePromise& promise) {
  TRACE_EVENT0("io", "IoDelegateImpl::OpenFile");
  TRACE_EVENT_WITH_FLOW1("promise", "Promise", promise.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::OpenFile");
  const auto& pair = FileIoContext::Open(file_name, mode);
  if (pair.second)
    return Reject(promise.reject, pair.second);
  const auto file = new FileIoContext(pair.first);
  const auto file_id = domapi::IoContextId::New();
  context_map_.emplace(file_id, file);
  RunCallback(base::Bind(promise.resolve, domapi::FileId(file_id)));
}

void IoDelegateImpl::OpenProcess(const base::string16& command_line,
                                 const domapi::OpenProcessPromise& promise) {
  TRACE_EVENT0("io", "IoDelegateImpl::OpenProcess");
  TRACE_EVENT_WITH_FLOW1("promise", "Promise", promise.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::OpenProcess");
  const auto process_id = domapi::IoContextId::New();
  const auto process = new ProcessIoContext(process_id, command_line, promise);
  context_map_.insert(std::make_pair(process_id, process));
  // Note: |ProcessIoContext| constructor delegate promise resolution to the
  // gateway thread which spawns process with specified command line.
}

void IoDelegateImpl::OpenWinResource(
    const base::string16& file_name,
    const domapi::OpenWinResourcePromise& promise) {
  const auto& pair = WinResourceIoContext::Open(file_name);
  if (pair.second)
    return Reject(promise.reject, pair.second);
  const auto resource_id = domapi::IoContextId::New();
  const auto resource = new WinResourceIoContext(pair.first);
  context_map_.emplace(resource_id, resource);
  RunCallback(base::Bind(promise.resolve, domapi::WinResourceId(resource_id)));
}

void IoDelegateImpl::QueryFileStatus(
    const base::string16& file_name,
    const domapi::QueryFileStatusPromise& promise) {
  TRACE_EVENT0("io", "IoDelegateImpl::QueryFileStatus");
  TRACE_EVENT_WITH_FLOW1("promise", "Promise", promise.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::QueryFileStatus");
  WIN32_FIND_DATAW find_data;
  scoped_find_handle find_handle(
      ::FindFirstFileW(file_name.c_str(), &find_data));
  if (!find_handle.is_valid()) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "FindFirstFileW failed";
    Reject(promise.reject, last_error);
    return;
  }

  if (find_data.nFileSizeHigh || find_data.nFileSizeLow > kHugeFileSize) {
    Reject(promise.reject, ERROR_NOT_ENOUGH_MEMORY);
    return;
  }

  domapi::FileStatus data = {0};
  data.file_size = static_cast<int>(find_data.nFileSizeLow);
  data.is_directory =
      (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  data.is_symlink =
      (find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
  data.last_write_time = base::Time::FromFileTime(find_data.ftLastWriteTime);
  data.readonly = (find_data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;

  RunCallback(base::Bind(promise.resolve, data));
}

void IoDelegateImpl::ReadDirectory(
    domapi::IoContextId context_id,
    size_t num_read,
    const domapi::ReadDirectoryPromise& promise) {
  const auto context = IoContextOf(context_id)->as<DirectoryIoContext>();
  if (!context)
    return Reject(promise.reject, ERROR_INVALID_HANDLE);
  context->Read(num_read, promise);
}

void IoDelegateImpl::ReadFile(domapi::IoContextId context_id,
                              void* buffer,
                              size_t num_read,
                              const domapi::IoIntPromise& promise) {
  TRACE_EVENT0("io", "IoDelegateImpl::ReadFile");
  const auto context = IoContextOf(context_id)->as<BlockIoContext>();
  if (!context)
    return Reject(promise.reject, ERROR_INVALID_HANDLE);
  context->Read(buffer, num_read, promise);
}

void IoDelegateImpl::RemoveFile(const base::string16& file_name,
                                const domapi::IoBoolPromise& resolver) {
  TRACE_EVENT0("io", "IoDelegateImpl::RemoveFile");
  const auto succeeded = ::DeleteFileW(file_name.c_str());
  if (!succeeded) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "DeleteFileEx failed";
    Reject(resolver.reject, last_error);
    return;
  }
  RunCallback(base::Bind(resolver.resolve, true));
}

void IoDelegateImpl::WriteFile(domapi::IoContextId context_id,
                               void* buffer,
                               size_t num_write,
                               const domapi::IoIntPromise& promise) {
  TRACE_EVENT0("io", "IoDelegateImpl::WriteFile");
  const auto context = IoContextOf(context_id)->as<BlockIoContext>();
  if (!context)
    return Reject(promise.reject, ERROR_INVALID_HANDLE);
  context->Write(buffer, num_write, promise);
}

}  // namespace io
