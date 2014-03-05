// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_os_file_handle_h)
#define INCLUDE_evita_dom_os_file_handle_h

#include "evita/v8_glue/scriptable.h"

namespace dom {

class FileHandle : public v8_glue::Scriptable<FileHandle> {
  private: closed_;
  private: domapi::IoHandle handle_;

  public: FileHandle(HANDLE handle);
  public: virtual ~FileHandle();

  public: void Close();
  public: v8::Handle<Object> ReadTo(
      v8::Handle<v8::ArrayBufferView> array_buffer_view);
  public: v8::Handle<Object> WriteFrom(
      v8::Handle<v8::ArrayBufferView> array_buffer_view);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_os_file_handle_h)
