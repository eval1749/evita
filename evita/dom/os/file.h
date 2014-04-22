// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_os_file_h)
#define INCLUDE_evita_dom_os_file_h

#include "evita/dom/os/abstract_file.h"

namespace domapi {
struct FileStatus;
}

namespace dom {

class MoveFileOptions;

namespace bindings {
class FileClass;
}

class File final : public v8_glue::Scriptable<File, AbstractFile> {
  DECLARE_SCRIPTABLE_OBJECT(File);
  friend class bindings::FileClass;

  public: explicit File(domapi::IoContextId context_id);
  public: virtual ~File();

  private: static v8::Handle<v8::Promise> MakeTempFileName(
      const base::string16& dir_name, const base::string16& prefix);

  // Move |src_path| to |dst_path|
  private: static v8::Handle<v8::Promise> Move(
      const base::string16& src_path,
      const base::string16& dst_path,
      const MoveFileOptions& options);
  private: static v8::Handle<v8::Promise> Move(
      const base::string16& src_path,
      const base::string16& dst_path);

  private: static v8::Handle<v8::Promise> Open(
      const base::string16& file_name,
      const base::string16& opt_mode);
  private: static v8::Handle<v8::Promise> Open(
      const base::string16& file_name);

  // Remove a existing |file_name|
  private: static v8::Handle<v8::Promise> Remove(
      const base::string16& file_name);

  private: static v8::Handle<v8::Promise> Stat(
      const base::string16& file_name);

  DISALLOW_COPY_AND_ASSIGN(File);
};

}  // namespace dom

#include "evita/dom/public/io_callback.h"

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

template<>
struct Converter<domapi::MoveFileOptions> {
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
      domapi::MoveFileOptions* out_options);
};
}  // namespace gin

#endif //!defined(INCLUDE_evita_dom_os_file_h)
