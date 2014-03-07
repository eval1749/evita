// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_common_win_scoped_handle_h)
#define INCLUDE_common_win_scoped_handle_h

namespace common {
namespace win {
class scoped_handle {
  private: HANDLE handle_;

  public: scoped_handle(const scoped_handle& other) = delete;
  public: scoped_handle(scoped_handle&& other)
      : handle_(other.handle_) {
    other.handle_ = INVALID_HANDLE_VALUE;
  }
  public: scoped_handle(HANDLE handle) : handle_(handle) {
  }
  public: scoped_handle() : scoped_handle(INVALID_HANDLE_VALUE) {
  }
  public: ~scoped_handle() {
    if (handle_ != INVALID_HANDLE_VALUE)
      ::CloseHandle(handle_);
  }

  public: operator bool() const { return is_valid(); }

  public: scoped_handle& operator=(const scoped_handle&) = delete;
  public: scoped_handle& operator=(scoped_handle&& other) {
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

#endif //!defined(INCLUDE_common_win_scoped_handle_h)
