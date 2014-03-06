// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/io/io_delegate_impl.h"

#include "base/bind.h"
#include "base/callback.h"
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/message_loop/message_pump_win.h"
#pragma warning(pop)
#include "base/time/time.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/io/io_manager.h"

#define DVLOG_WIN32_ERROR(level, name) \
  DVLOG(level) << name ": " << this << " " << file_name << " err=" << dwError

namespace common {
namespace win {
class GenericScopedHandle {
  private: HANDLE handle_;

  public: GenericScopedHandle(const GenericScopedHandle& other) = delete;
  public: GenericScopedHandle(GenericScopedHandle&& other)
      : handle_(other.handle_) {
    other.handle_ = INVALID_HANDLE_VALUE;
  }
  public: GenericScopedHandle(HANDLE handle) : handle_(handle) {
  }
  public: GenericScopedHandle() : GenericScopedHandle(INVALID_HANDLE_VALUE) {
  }
  public: ~GenericScopedHandle() {
    if (handle_ != INVALID_HANDLE_VALUE)
      ::CloseHandle(handle_);
  }

  public: operator bool() const { return is_valid(); }

  public: GenericScopedHandle& operator=(const GenericScopedHandle&) = delete;
  public: GenericScopedHandle& operator=(GenericScopedHandle&& other) {
    DCHECK_EQ(INVALID_HANDLE_VALUE, handle_);
    handle_ = other.handle_;
    other.handle_ = INVALID_HANDLE_VALUE;
    return *this;
  }

  public: HANDLE get() const { return handle_; }
  public: bool is_valid() const { return handle_ != INVALID_HANDLE_VALUE; }

  public: HANDLE release() {
    DCHECK(is_valid());
    auto const handle = handle_;
    handle_ = INVALID_HANDLE_VALUE;
    return handle;
  }
};
}  // namespace win
}  // namespace common

namespace io {

namespace {
const DWORD kHugeFileSize = 1u << 28;

domapi::IoHandle* FileHandleToDomIoHandle(HANDLE handle) {
  return reinterpret_cast<domapi::IoHandle*>(handle);
}

HANDLE DomIoHandleToFileHandle(domapi::IoHandle* handle) {
  return reinterpret_cast<HANDLE>(handle);
}


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
  };

  public: IoHandler();
  public: virtual ~IoHandler();

  private: IoContextImpl* CreateIoContext(
      const IoDelegate::FileIoCallback& callback);
  public: void ReadFile(HANDLE file_handle, void* buffer, size_t num_read,
                       const IoDelegate::FileIoCallback& callback);
  public: void WriteFile(HANDLE file_handle, void* buffer, size_t num_write,
                         const IoDelegate::FileIoCallback& callback);

  // base::MessagePumpForIO::IOHandler
  private: virtual void OnIOCompleted(
      IOContext* context, DWORD bytes_transfered, DWORD error) override;
};

IoDelegateImpl::IoHandler::IoHandler() {
}

IoDelegateImpl::IoHandler::~IoHandler() {
}

IoDelegateImpl::IoHandler::IoContextImpl*
    IoDelegateImpl::IoHandler::CreateIoContext(
        const domapi::IoDelegate::FileIoCallback& callback) {
  auto const context = new IoContextImpl();
  context->handler = this;
  context->callback = callback;
  return context;
}

void IoDelegateImpl::IoHandler::ReadFile(HANDLE file_handle, void* buffer,
    size_t num_read, const domapi::IoDelegate::FileIoCallback& callback) {
  auto const context = CreateIoContext(callback);
  DWORD read;
  auto const succeeded = ::ReadFile(file_handle, buffer, num_read,
                                    &read, &context->overlapped);
  if (succeeded)
    return;
  auto const error = ::GetLastError();
  if (error == ERROR_IO_PENDING)
    return;
  if (error == ERROR_HANDLE_EOF) {
    OnIOCompleted(context, 0, 0);
    return;
  }
  OnIOCompleted(context, 0, error);
}

void IoDelegateImpl::IoHandler::WriteFile(HANDLE file_handle, void* buffer,
    size_t num_write, const domapi::IoDelegate::FileIoCallback& callback) {
  auto const context = CreateIoContext(callback);
  DWORD written;
  auto const succeeded = ::WriteFile(file_handle, buffer, num_write, &written,
                                     &context->overlapped);
  if (succeeded)
    return;
  auto const error = ::GetLastError();
  if (error == ERROR_IO_PENDING)
    return;
  OnIOCompleted(context, 0, error);
}

// base::MessagePumpForIO::IoHandler
void IoDelegateImpl::IoHandler::OnIOCompleted(IOContext* context,
                                              DWORD bytes_transfered,
                                              DWORD error) {
  static_cast<IoContextImpl*>(context)->callback.Run(
      static_cast<int>(bytes_transfered), static_cast<int>(error));
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
void IoDelegateImpl::CloseFile(domapi::IoHandle* io_handle) {
  auto const file_handle = DomIoHandleToFileHandle(io_handle);
  if (file_handle != INVALID_HANDLE_VALUE)
    ::CloseHandle(file_handle);
}

void IoDelegateImpl::OpenFile(const base::string16& file_name,
                              const base::string16& mode,
                              const OpenFileCallback& callback) {
  CreateFileParams params(mode);
  common::win::GenericScopedHandle handle = ::CreateFileW(file_name.c_str(),
      params.access, params.share_mode, nullptr, params.creation,
      FILE_FLAG_OVERLAPPED, nullptr);
  if (!handle) {
    auto const error_code = ::GetLastError();
    Application::instance()->view_event_handler()->RunCallback(
        base::Bind(callback, static_cast<domapi::IoHandle*>(nullptr),
                   static_cast<int>(error_code)));
    return;
  }
  Application::instance()->GetIoManager()->RegisterIoHandler(
      handle.get(), io_handler_.get());
  Application::instance()->view_event_handler()->RunCallback(
      base::Bind(callback, FileHandleToDomIoHandle(handle.release()), 0));
}

void IoDelegateImpl::QueryFileStatus(const base::string16& file_name,
                                     const QueryFileStatusCallback& callback) {
  QueryFileStatusHandler handler(file_name);
  Application::instance()->view_event_handler()->RunCallback(
      base::Bind(callback, handler.data()));
}

void IoDelegateImpl::ReadFile(domapi::IoHandle* io_handle, void* buffer,
                              size_t num_read,
                              const FileIoCallback& callback) {
  io_handler_->ReadFile(DomIoHandleToFileHandle(io_handle), buffer, num_read,
                        callback);
}

void IoDelegateImpl::WriteFile(domapi::IoHandle* io_handle, void* buffer,
                               size_t num_write,
                               const FileIoCallback& callback) {
  io_handler_->WriteFile(DomIoHandleToFileHandle(io_handle), buffer, num_write,
                         callback);
}

}  // namespace io
