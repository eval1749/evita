// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_WIN_SCOPED_HANDLE_H_
#define COMMON_WIN_SCOPED_HANDLE_H_

#include <windows.h>

#include "base/logging.h"

namespace common {
namespace win {
class scoped_handle final {
 public:
  scoped_handle(const scoped_handle& other) = delete;
  scoped_handle(scoped_handle&& other) : handle_(other.handle_) {
    other.handle_ = INVALID_HANDLE_VALUE;
  }

  explicit scoped_handle(HANDLE handle) : handle_(handle) {}
  scoped_handle() : scoped_handle(INVALID_HANDLE_VALUE) {}
  ~scoped_handle() {
    if (handle_ != INVALID_HANDLE_VALUE)
      ::CloseHandle(handle_);
  }
  explicit operator bool() const { return is_valid(); }

  scoped_handle& operator=(const scoped_handle&) = delete;
  scoped_handle& operator=(scoped_handle&& other) {
    DCHECK(!is_valid());
    handle_ = other.handle_;
    other.handle_ = INVALID_HANDLE_VALUE;
    return *this;
  }

  HANDLE get() const { return handle_; }
  bool is_valid() const { return handle_ != INVALID_HANDLE_VALUE; }

  HANDLE* location() {
    DCHECK(!is_valid());
    return &handle_;
  }

  HANDLE release() {
    DCHECK(is_valid());
    auto const handle = handle_;
    handle_ = INVALID_HANDLE_VALUE;
    return handle;
  }

  void reset(HANDLE new_handle) {
    DCHECK(!is_valid());
    handle_ = new_handle;
  }

 private:
  HANDLE handle_;
};

}  // namespace win
}  // namespace common

#endif  // COMMON_WIN_SCOPED_HANDLE_H_
