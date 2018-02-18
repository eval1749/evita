// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include <algorithm>

#include "evita/io/win_resource_io_context.h"

#include "base/callback.h"
#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "base/strings/stringprintf.h"
#include "evita/dom/public/io_callback.h"
#include "evita/dom/public/io_error.h"
#include "evita/dom/public/promise.h"
#include "evita/io/io_context_utils.h"

namespace io {

namespace {

//////////////////////////////////////////////////////////////////////
//
// ResourceNameEnumerator
//
class ResourceNameEnumerator final {
 public:
  ResourceNameEnumerator() = default;
  ~ResourceNameEnumerator() = default;

  std::pair<std::vector<base::string16>, int> Run(HMODULE module,
                                                  const base::string16& type);

 private:
  void Add(base::StringPiece16 name) { names_.emplace_back(name.as_string()); }

  static BOOL CALLBACK EnumProc(HMODULE hModuel,
                                LPCWSTR type,
                                LPWSTR name,
                                LONG_PTR param) {
    const auto enumerator = reinterpret_cast<ResourceNameEnumerator*>(param);
    if (IS_INTRESOURCE(name)) {
      const auto id = static_cast<int>(reinterpret_cast<LONG_PTR>(name));
      enumerator->Add(base::StringPrintf(L"#%d", id));
      return TRUE;
    }
    enumerator->Add(base::StringPiece16(name).as_string());
    return TRUE;
  }

  std::vector<base::string16> names_;

  DISALLOW_COPY_AND_ASSIGN(ResourceNameEnumerator);
};

std::pair<std::vector<base::string16>, int> ResourceNameEnumerator::Run(
    HMODULE module,
    const base::string16& type) {
  names_.clear();
  const auto succeeded = ::EnumResourceNamesEx(
      module, type.c_str(), ResourceNameEnumerator::EnumProc,
      reinterpret_cast<LONG_PTR>(this), RESOURCE_ENUM_VALIDATE, 0);
  if (!succeeded) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "EnumResourceNamesEx failed";
    return std::make_pair(names_, last_error);
  }
  return std::make_pair(names_, 0);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// WinResourceIoContext
//
WinResourceIoContext::WinResourceIoContext(HMODULE module) : module_(module) {}

WinResourceIoContext::~WinResourceIoContext() {}

std::pair<std::vector<base::string16>, int>
WinResourceIoContext::GetResourceName(const base::string16& type) {
  return ResourceNameEnumerator().Run(module_, type);
}

std::pair<int, int> WinResourceIoContext::Load(const base::string16& type,
                                               const base::string16& name,
                                               uint8_t* buffer,
                                               size_t buffer_size) {
  const auto resource =
      ::FindResourceEx(module_, type.c_str(), name.c_str(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
  if (!resource) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "FindResourceEx failed";
    return std::make_pair(0, last_error);
  }
  const auto data_handle = ::LoadResource(module_, resource);
  if (!data_handle) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "LoadResourceEx failed";
    return std::make_pair(0, last_error);
  }
  const auto resource_size =
      static_cast<size_t>(::SizeofResource(module_, resource));
  if (resource_size == 0) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "SizeofResource failed";
    return std::make_pair(0, last_error);
  }
  const auto resource_bytes = ::LockResource(data_handle);
  if (!resource_bytes) {
    const auto last_error = ::GetLastError();
    PLOG(ERROR) << "LockResource failed";
    return std::make_pair(0, last_error);
  }
  ::memcpy(buffer, resource_bytes, std::min(resource_size, buffer_size));
  return std::make_pair(static_cast<int>(resource_size), 0);
}

std::pair<HMODULE, int> WinResourceIoContext::Open(
    const base::string16& file_name) {
  const auto handle = ::LoadLibraryEx(
      file_name.c_str(), nullptr,
      LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
  if (handle)
    return std::make_pair(handle, 0);
  const auto last_error = ::GetLastError();
  PLOG(ERROR) << "LoadLibraryEx failed";
  return std::make_pair(nullptr, last_error);
}

// io::IoContext
void WinResourceIoContext::Close(domapi::IoIntPromise promise) {
  if (!module_)
    return Resolve(std::move(promise.resolve), 0);
  if (::FreeLibrary(module_))
    return Resolve(std::move(promise.resolve), 0);
  const auto last_error = ::GetLastError();
  PLOG(ERROR) << "FreeLibrary failed";
  return Reject(std::move(promise.reject), last_error);
}

}  // namespace io
