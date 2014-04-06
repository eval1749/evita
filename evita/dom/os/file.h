// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_os_file_h)
#define INCLUDE_evita_dom_os_file_h

#include "evita/dom/os/abstract_file.h"

#include "evita/v8_glue/optional.h"

namespace domapi {
struct FileStatus;
}

namespace dom {

class File : public v8_glue::Scriptable<File, AbstractFile> {
  DECLARE_SCRIPTABLE_OBJECT(File);

  public: File(domapi::IoContextId context_id);
  public: virtual ~File();

  public: static v8::Handle<v8::Promise> MakeTempFileName(
      const base::string16& dir_name, const base::string16& prefix);
  public: static v8::Handle<v8::Promise> Open(
      const base::string16& file_name,
      v8_glue::Optional<base::string16> opt_mode);
  public: static v8::Handle<v8::Promise> Stat(
      const base::string16& filename);

  DISALLOW_COPY_AND_ASSIGN(File);
};

}  // namespace dom

namespace gin {
template<>
struct Converter<domapi::FileId> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      domapi::FileId context_id);
};

template<>
struct Converter<domapi::FileStatus> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
        const domapi::FileStatus& data);
};
}  // namespace gin

#endif //!defined(INCLUDE_evita_dom_os_file_h)
