// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/io_delegate_impl.h"

#include <unordered_map>

#include "base/bind.h"
#include "base/callback.h"
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/message_loop/message_pump_win.h"
#pragma warning(pop)
#include "base/time/time.h"
#include "common/win/scoped_handle.h"
#include "evita/dom/public/io_context_id.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/io/io_manager.h"

#define DVLOG_WIN32_ERROR(level, name) \
  DVLOG(level) << name ": " << this << " " << file_name << " err=" << dwError

namespace io {

namespace {
const DWORD kHugeFileSize = 1u << 28;

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

//////////////////////////////////////////////////////////////////////
//
// QueryFileStatusHandler
//
class QueryFileStatusHandler {
  private: domapi::QueryFileStatusCallbackData data_;
  private: HANDLE find_handle_;

  public: QueryFileStatusHandler(const base::string16& file_name);
  public: ~QueryFileStatusHandler();

  public: const domapi::QueryFileStatusCallbackData& data() const {
    return data_;
  }

  DISALLOW_COPY_AND_ASSIGN(QueryFileStatusHandler);
};

QueryFileStatusHandler::QueryFileStatusHandler(
    const base::string16& file_name) {
  data_ = {0};

  WIN32_FIND_DATAW find_data;
  find_handle_ = ::FindFirstFileW(file_name.c_str(), &find_data);
  if (find_handle_ == INVALID_HANDLE_VALUE) {
    auto const dwError = ::GetLastError();
    DVLOG_WIN32_ERROR(0, "FindFirstFileW");
    data_.error_code = static_cast<int>(dwError);
    return;
  }

  if (find_data.nFileSizeHigh || find_data.nFileSizeLow > kHugeFileSize) {
    data_.error_code = static_cast<int>(ERROR_NOT_ENOUGH_MEMORY);
    return ;
  }

  data_.error_code = 0;
  data_.file_size = static_cast<int>(find_data.nFileSizeLow);
  data_.is_directory = find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
  data_.is_symlink = find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
  data_.last_write_time = base::Time::FromFileTime(find_data.ftLastWriteTime);
  data_.readonly = find_data.dwFileAttributes & FILE_ATTRIBUTE_READONLY;
}

QueryFileStatusHandler::~QueryFileStatusHandler() {
  if (find_handle_ != INVALID_HANDLE_VALUE) {
    ::FindClose(find_handle_);
  }
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// IoDelegateImpl::IoHandler
//
class IoDelegateImpl::IoHandler : public base::MessagePumpForIO::IOHandler {
  private: typedef base::MessagePumpForIO::IOContext IOContext;

  private: struct IoContextImpl : IOContext {
    domapi::IoDelegate::FileIoCallback callback;
    HANDLE file_handle;
    bool running;
  };

  private: typedef std::unordered_map<domapi::IoContextId, IoContextImpl*>
      ContextMap;

  private: ContextMap context_map_;

  public: IoHandler();
  public: virtual ~IoHandler();

  public: void CloseFile(domapi::IoContextId context_id);
  private: IoContextImpl* FindContext(domapi::IoContextId context_id) const;
  public: void ReadFile(domapi::IoContextId context_id, void* buffer,
                        size_t num_read,
                        const IoDelegate::FileIoCallback& callback);
  public: domapi::IoContextId Register(HANDLE file_handle);
  public: void WriteFile(domapi::IoContextId context_id, void* buffer,
                         size_t num_write,
                         const IoDelegate::FileIoCallback& callback);

  // base::MessagePumpForIO::IOHandler
  private: virtual void OnIOCompleted(
      IOContext* context, DWORD bytes_transfered, DWORD error) override;
};

IoDelegateImpl::IoHandler::IoHandler() {
}

IoDelegateImpl::IoHandler::~IoHandler() {
}

void IoDelegateImpl::IoHandler::CloseFile(domapi::IoContextId context_id) {
  auto it = context_map_.find(context_id);
  if (it == context_map_.end())
    return;
  auto const context = it->second;
  if (context->file_handle != INVALID_HANDLE_VALUE) {
    context->file_handle = INVALID_HANDLE_VALUE;
    ::CloseHandle(context->file_handle);
  }
  context_map_.erase(it);
  if (!context->running)
    delete context;
}

IoDelegateImpl::IoHandler::IoContextImpl*
    IoDelegateImpl::IoHandler::FindContext(
        domapi::IoContextId context_id) const {
  auto it = context_map_.find(context_id);
  return it == context_map_.end() ? nullptr : it->second;
}

void IoDelegateImpl::IoHandler::ReadFile(domapi::IoContextId context_id,
    void* buffer, size_t num_read,
    const domapi::IoDelegate::FileIoCallback& callback) {
  auto const context = FindContext(context_id);
  if (!context) {
    Application::instance()->view_event_handler()->RunCallback(
        base::Bind(callback, 0, ERROR_INVALID_HANDLE));
    return;
  }

  if (context->running) {
    Application::instance()->view_event_handler()->RunCallback(
        base::Bind(callback, 0, ERROR_BUSY));
    return;
  }

  context->callback = callback;
  DWORD read;
  auto const succeeded = ::ReadFile(context->file_handle, buffer, num_read,
                                    &read, &context->overlapped);
  DCHECK(!succeeded);

  auto const error = ::GetLastError();
  if (error == ERROR_IO_PENDING) {
    context->running = true;
    return;
  }
  OnIOCompleted(context, 0, error);
}

domapi::IoContextId IoDelegateImpl::IoHandler::Register(HANDLE file_handle) {
  auto const context = new IoContextImpl();
  context->file_handle = file_handle;
  context->handler = this;
  context->overlapped = {0};
  context->running = false;
  auto const context_id = domapi::IoContextId::New();
  context_map_[context_id] = context;
  return context_id;
}

void IoDelegateImpl::IoHandler::WriteFile(domapi::IoContextId context_id,
    void* buffer, size_t num_write,
    const domapi::IoDelegate::FileIoCallback& callback) {
  auto const context = FindContext(context_id);
  if (!context) {
    Application::instance()->view_event_handler()->RunCallback(
        base::Bind(callback, 0, ERROR_INVALID_HANDLE));
    return;
  }

  if (context->running) {
    Application::instance()->view_event_handler()->RunCallback(
        base::Bind(callback, 0, ERROR_BUSY));
    return;
  }

  context->callback = callback;
  DWORD written;
  auto const succeeded = ::WriteFile(context->file_handle, buffer, num_write,
                                     &written, &context->overlapped);
  DCHECK(!succeeded);
  auto const error = ::GetLastError();
  if (error == ERROR_IO_PENDING) {
    context->running = true;
    return;
  }
  OnIOCompleted(context, 0, error);
}

// base::MessagePumpForIO::IoHandler
void IoDelegateImpl::IoHandler::OnIOCompleted(IOContext* io_context,
                                              DWORD bytes_transfered,
                                              DWORD error) {
  auto const context = static_cast<IoContextImpl*>(io_context);
  if (!error)
    context->overlapped.Offset += bytes_transfered;
  context->running = false;
  if (error == ERROR_HANDLE_EOF) {
    Application::instance()->view_event_handler()->RunCallback(
        base::Bind(context->callback, 0, 0));
  } else {
    Application::instance()->view_event_handler()->RunCallback(
        base::Bind(context->callback,
                   static_cast<int>(bytes_transfered),
                   static_cast<int>(error)));
  }
  if (context->file_handle == INVALID_HANDLE_VALUE)
    delete context;
}

//////////////////////////////////////////////////////////////////////
//
// IoDelegateImpl
//
IoDelegateImpl::IoDelegateImpl() : io_handler_(new IoHandler()) {
}

IoDelegateImpl::~IoDelegateImpl() {
}

// domapi::IoDelegate
void IoDelegateImpl::CloseFile(domapi::IoContextId context_id) {
  io_handler_->CloseFile(context_id);
}

void IoDelegateImpl::OpenFile(const base::string16& file_name,
                              const base::string16& mode,
                              const OpenFileCallback& callback) {
  CreateFileParams params(mode);
  common::win::scoped_handle handle = ::CreateFileW(file_name.c_str(),
      params.access, params.share_mode, nullptr, params.creation,
      FILE_FLAG_OVERLAPPED, nullptr);
  if (!handle) {
    auto const error_code = ::GetLastError();
    Application::instance()->view_event_handler()->RunCallback(
        base::Bind(callback, domapi::IoContextId(),
                   static_cast<int>(error_code)));
    return;
  }
  Application::instance()->GetIoManager()->RegisterIoHandler(
      handle.get(), io_handler_.get());
  Application::instance()->view_event_handler()->RunCallback(
      base::Bind(callback, io_handler_->Register(handle.release()), 0));
}

void IoDelegateImpl::QueryFileStatus(const base::string16& file_name,
                                     const QueryFileStatusCallback& callback) {
  QueryFileStatusHandler handler(file_name);
  Application::instance()->view_event_handler()->RunCallback(
      base::Bind(callback, handler.data()));
}

void IoDelegateImpl::ReadFile(domapi::IoContextId context_id, void* buffer,
                              size_t num_read,
                              const FileIoCallback& callback) {
  io_handler_->ReadFile(context_id, buffer, num_read, callback);
}

void IoDelegateImpl::WriteFile(domapi::IoContextId context_id, void* buffer,
                               size_t num_write,
                               const FileIoCallback& callback) {
  io_handler_->WriteFile(context_id, buffer, num_write, callback);
}

}  // namespace io
