// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/io/win_resource_io_context.h"

#include "base/callback.h"
#include "base/logging.h"
#include "evita/dom/public/io_callback.h"
#include "evita/dom/public/io_error.h"
#include "evita/dom/public/promise.h"
#include "evita/io/io_context_utils.h"

namespace io {

//////////////////////////////////////////////////////////////////////
//
// WinResourceIoContext
//
WinResourceIoContext::WinResourceIoContext(
    const domapi::IoContextId& context_id,
    HMODULE module)
    : context_id_(context_id), module_(module) {}

WinResourceIoContext::~WinResourceIoContext() {}

std::pair<int, int> WinResourceIoContext::Load(const base::string16& type,
                                               const base::string16& name,
                                               uint8_t* buffer,
                                               size_t buffer_size) {
  const auto resource =
      ::FindResourceEx(module_, type.c_str(), name.c_str(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
  if (!resource) {
    const auto last_error = ::GetLastError();
    DVLOG(0) << "FindResourceEx error=" << last_error;
    return std::make_pair(0, last_error);
  }
  const auto data_handle = ::LoadResource(module_, resource);
  if (!data_handle) {
    const auto last_error = ::GetLastError();
    DVLOG(0) << "LoadResourceEx error=" << last_error;
    return std::make_pair(0, last_error);
  }
  const auto resource_size =
      static_cast<size_t>(::SizeofResource(module_, resource));
  if (resource_size == 0) {
    const auto last_error = ::GetLastError();
    DVLOG(0) << "SizeofResource error=" << last_error;
    return std::make_pair(0, last_error);
  }
  const auto resource_bytes = ::LockResource(data_handle);
  if (!resource_bytes) {
    const auto last_error = ::GetLastError();
    DVLOG(0) << "LockResource error=" << last_error;
    return std::make_pair(0, last_error);
  }
  ::memcpy(buffer, resource_bytes, std::min(resource_size, buffer_size));
  return std::make_pair(resource_size, 0);
}

std::pair<HMODULE, int> WinResourceIoContext::Open(
    const base::string16& file_name) {
  const auto handle = ::LoadLibraryEx(
      file_name.c_str(), nullptr,
      LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
  if (handle)
    return std::make_pair(handle, 0);
  const auto last_error = ::GetLastError();
  DVLOG(0) << "LoadLibraryEx error=" << last_error;
  return std::make_pair(nullptr, last_error);
}

// io::IoContext
void WinResourceIoContext::Close(const domapi::IoIntPromise& promise) {
  if (!module_)
    return Resolve(promise.resolve, 0);
  if (::FreeLibrary(module_))
    return Resolve(promise.resolve, 0);
  const auto last_error = ::GetLastError();
  DVLOG(0) << "FreeLibrary error=" << last_error;
  return Reject(promise.reject, last_error);
}

}  // namespace io
