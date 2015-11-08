// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/io_delegate_impl.h"

#include <unordered_map>

#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_pump_win.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"
#include "common/win/scoped_handle.h"
#include "evita/dom/public/deferred.h"
#include "evita/dom/public/io_context_id.h"
#include "evita/dom/public/io_error.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/io/file_io_context.h"
#include "evita/io/io_context_utils.h"
#include "evita/io/process_io_context.h"
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

void Resolve(const base::Callback<void(domapi::FileId)>& resolve,
             domapi::FileId context_id) {
  editor::Application::instance()->view_event_handler()->RunCallback(
      base::Bind(resolve, context_id));
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// IoDelegateImpl
//
IoDelegateImpl::IoDelegateImpl() {}

IoDelegateImpl::~IoDelegateImpl() {}

// domapi::IoDelegate
void IoDelegateImpl::CheckSpelling(const base::string16& word_to_check,
                                   const CheckSpellingResolver& deferred) {
  TRACE_EVENT_WITH_FLOW1("script", "Promise", deferred.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::CheckSpelling");
  editor::Application::instance()->view_event_handler()->RunCallback(base::Bind(
      deferred.resolve,
      spellchecker::SpellingEngine::GetSpellingEngine()->CheckSpelling(
          word_to_check)));
}

void IoDelegateImpl::CloseFile(domapi::IoContextId context_id,
                               const domapi::FileIoDeferred& deferred) {
  auto const it = context_map_.find(context_id);
  if (it == context_map_.end()) {
    return;
  }
  it->second->Close(deferred);
  context_map_.erase(it);
}

void IoDelegateImpl::GetSpellingSuggestions(
    const base::string16& wrong_word,
    const GetSpellingSuggestionsResolver& deferred) {
  TRACE_EVENT_WITH_FLOW1("script", "Promise", deferred.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::GetSpellingSuggestions");
  editor::Application::instance()->view_event_handler()->RunCallback(base::Bind(
      deferred.resolve,
      spellchecker::SpellingEngine::GetSpellingEngine()->GetSpellingSuggestions(
          wrong_word)));
}

void IoDelegateImpl::MakeTempFileName(
    const base::string16& dir_name,
    const base::string16& prefix,
    const domapi::MakeTempFileNameResolver& resolver) {
  TRACE_EVENT_WITH_FLOW1("script", "Promise", resolver.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::MakeTempFileName");
  base::string16 file_name(MAX_PATH, 0);
  auto const unique_id =
      ::GetTempFileNameW(dir_name.c_str(), prefix.c_str(), 0, &file_name[0]);
  if (!unique_id) {
    auto const last_error = ::GetLastError();
    DVLOG(0) << "GetTempFileNameW error=" << last_error;
    Reject(resolver.reject, last_error);
    return;
  }
  auto const nul_pos = file_name.find(static_cast<base::char16>(0));
  if (nul_pos != base::string16::npos)
    file_name.resize(nul_pos);
  editor::Application::instance()->view_event_handler()->RunCallback(
      base::Bind(resolver.resolve, file_name));
}

void IoDelegateImpl::MoveFile(const base::string16& src_path,
                              const base::string16& dst_path,
                              const domapi::MoveFileOptions& options,
                              const domapi::IoResolver& resolver) {
  TRACE_EVENT0("io", "IoDelegateImpl::MoveFile");
  TRACE_EVENT_WITH_FLOW1("script", "Promise", resolver.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::MoveFile");
  auto const flags = options.no_overwrite
                         ? MOVEFILE_WRITE_THROUGH
                         : MOVEFILE_WRITE_THROUGH | MOVEFILE_REPLACE_EXISTING;
  auto const succeeded = ::MoveFileExW(src_path.c_str(), dst_path.c_str(),
                                       static_cast<uint32_t>(flags));
  if (!succeeded) {
    auto const last_error = ::GetLastError();
    DVLOG(0) << "MoveFileEx error=" << last_error;
    Reject(resolver.reject, last_error);
    return;
  }
  editor::Application::instance()->view_event_handler()->RunCallback(
      base::Bind(resolver.resolve, true));
}

void IoDelegateImpl::OpenFile(const base::string16& file_name,
                              const base::string16& mode,
                              const domapi::OpenFileDeferred& deferred) {
  TRACE_EVENT0("io", "IoDelegateImpl::OpenFile");
  TRACE_EVENT_WITH_FLOW1("script", "Promise", deferred.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::OpenFile");
  auto file = std::make_unique<FileIoContext>(file_name, mode, deferred);
  if (!file->is_valid())
    return;
  auto const file_id = domapi::IoContextId::New();
  context_map_[file_id] = file.release();
  Resolve(deferred.resolve, domapi::FileId(file_id));
}

void IoDelegateImpl::OpenProcess(const base::string16& command_line,
                                 const domapi::OpenProcessDeferred& deferred) {
  TRACE_EVENT0("io", "IoDelegateImpl::OpenProcess");
  TRACE_EVENT_WITH_FLOW1("script", "Promise", deferred.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::OpenProcess");
  auto const process_id = domapi::IoContextId::New();
  auto const process = new ProcessIoContext(process_id, command_line, deferred);
  context_map_[process_id] = process;
}

void IoDelegateImpl::QueryFileStatus(
    const base::string16& file_name,
    const domapi::QueryFileStatusDeferred& deferred) {
  TRACE_EVENT0("io", "IoDelegateImpl::QueryFileStatus");
  TRACE_EVENT_WITH_FLOW1("script", "Promise", deferred.sequence_num,
                         TRACE_EVENT_FLAG_FLOW_IN | TRACE_EVENT_FLAG_FLOW_OUT,
                         "step", "IoDelegateImpl::QueryFileStatus");
  WIN32_FIND_DATAW find_data;
  scoped_find_handle find_handle(
      ::FindFirstFileW(file_name.c_str(), &find_data));
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
  data.is_directory =
      (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  data.is_symlink =
      (find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
  data.last_write_time = base::Time::FromFileTime(find_data.ftLastWriteTime);
  data.readonly = (find_data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;

  editor::Application::instance()->view_event_handler()->RunCallback(
      base::Bind(deferred.resolve, data));
}

void IoDelegateImpl::ReadFile(domapi::IoContextId context_id,
                              void* buffer,
                              size_t num_read,
                              const domapi::FileIoDeferred& deferred) {
  TRACE_EVENT0("io", "IoDelegateImpl::ReadFile");
  auto const it = context_map_.find(context_id);
  if (it == context_map_.end()) {
    Reject(deferred.reject, ERROR_INVALID_HANDLE);
    return;
  }
  it->second->Read(buffer, num_read, deferred);
}

void IoDelegateImpl::RemoveFile(const base::string16& file_name,
                                const domapi::IoResolver& resolver) {
  TRACE_EVENT0("io", "IoDelegateImpl::RemoveFile");
  auto const succeeded = ::DeleteFileW(file_name.c_str());
  if (!succeeded) {
    auto const last_error = ::GetLastError();
    DVLOG(0) << "DeleteFileEx error=" << last_error;
    Reject(resolver.reject, last_error);
    return;
  }
  editor::Application::instance()->view_event_handler()->RunCallback(
      base::Bind(resolver.resolve, true));
}

void IoDelegateImpl::WriteFile(domapi::IoContextId context_id,
                               void* buffer,
                               size_t num_write,
                               const domapi::FileIoDeferred& deferred) {
  TRACE_EVENT0("io", "IoDelegateImpl::WriteFile");
  auto const it = context_map_.find(context_id);
  if (it == context_map_.end()) {
    Reject(deferred.reject, ERROR_INVALID_HANDLE);
    return;
  }
  it->second->Write(buffer, num_write, deferred);
}

}  // namespace io
