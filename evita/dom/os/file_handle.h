// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_os_file_handle_h)
#define INCLUDE_evita_dom_os_file_handle_h

#include "evita/v8_glue/scriptable.h"

namespace domapi {
struct IoHandle;
}

namespace gin {
class ArrayBufferView;
}

namespace dom {

class FileHandle : public v8_glue::Scriptable<FileHandle> {
  DECLARE_SCRIPTABLE_OBJECT(FileHandle);

  private: bool closed_;
  private: domapi::IoHandle* handle_;

  public: FileHandle(domapi::IoHandle* handle);
  public: virtual ~FileHandle();

  public: void Close();
  public: void Read(const gin::ArrayBufferView& array_buffer_view,
                    v8::Handle<v8::Function> callback);
  public: void Write(const gin::ArrayBufferView& array_buffer_view,
                     v8::Handle<v8::Function> callback);

  DISALLOW_COPY_AND_ASSIGN(FileHandle);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_os_file_handle_h)
