// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_os_abstract_file_h)
#define INCLUDE_evita_dom_os_abstract_file_h

#include "evita/dom/public/io_context_id.h"
#include "evita/v8_glue/scriptable.h"

namespace gin {
class ArrayBufferView;
}

namespace dom {
namespace os {

class AbstractFile : public v8_glue::Scriptable<AbstractFile> {
  DECLARE_SCRIPTABLE_OBJECT(AbstractFile);

  private: domapi::IoContextId context_id_;

  protected: AbstractFile(domapi::IoContextId context_id);
  protected: virtual ~AbstractFile();

  public: v8::Handle<v8::Object> Close();
  public: v8::Handle<v8::Object> Read(
      const gin::ArrayBufferView& array_buffer_view);
  public: v8::Handle<v8::Object> Write(
      const gin::ArrayBufferView& array_buffer_view);

  DISALLOW_COPY_AND_ASSIGN(AbstractFile);
};

struct FileError {
  int error_code;
  FileError(int error_code) : error_code(error_code) {
  }
};

}  // namespace os
}  // namespace dom

namespace gin {
template<>
struct Converter<dom::os::FileError> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    const dom::os::FileError& error);
};
}  // namespace gin

#endif //!defined(INCLUDE_evita_dom_os_abstract_file_h)
