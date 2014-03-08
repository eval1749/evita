// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_os_file_h)
#define INCLUDE_evita_dom_os_file_h

#include "evita/dom/public/io_context_id.h"
#include "evita/v8_glue/scriptable.h"

namespace domapi {
struct IoHandle;
}

namespace gin {
class ArrayBufferView;
}

namespace dom {
namespace os {

class File : public v8_glue::Scriptable<File> {
  DECLARE_SCRIPTABLE_OBJECT(File);

  private: bool closed_;
  private: domapi::IoContextId context_id_;

  public: File(domapi::IoContextId context_id);
  public: virtual ~File();

  public: void Close();
  public: v8::Handle<v8::Object> Read(
      const gin::ArrayBufferView& array_buffer_view);
  public: v8::Handle<v8::Object> Write(
      const gin::ArrayBufferView& array_buffer_view);

  DISALLOW_COPY_AND_ASSIGN(File);
};

}  // namespace os
}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_os_file_h)
