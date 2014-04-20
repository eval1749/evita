// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_os_abstract_file_h)
#define INCLUDE_evita_dom_os_abstract_file_h

#include "evita/dom/public/io_context_id.h"
#include "evita/dom/public/io_error.h"
#include "evita/v8_glue/scriptable.h"

namespace gin {
class ArrayBufferView;
}

namespace dom {

namespace bindings {
class AbstractFileClass;
}

class AbstractFile : public v8_glue::Scriptable<AbstractFile> {
  DECLARE_SCRIPTABLE_OBJECT(AbstractFile);
  friend class bindings::AbstractFileClass;

  private: domapi::IoContextId context_id_;

  protected: AbstractFile(domapi::IoContextId context_id);
  protected: virtual ~AbstractFile();

  private: v8::Handle<v8::Promise> Close();
  private: v8::Handle<v8::Promise> Read(
      const gin::ArrayBufferView& array_buffer_view);
  private: v8::Handle<v8::Promise> Write(
      const gin::ArrayBufferView& array_buffer_view);

  DISALLOW_COPY_AND_ASSIGN(AbstractFile);
};

}  // namespace dom

namespace gin {
template<>
struct Converter<domapi::IoError> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    const domapi::IoError& error);
};
}  // namespace gin

#endif //!defined(INCLUDE_evita_dom_os_abstract_file_h)
