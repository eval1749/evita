// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_OS_FILE_H_
#define EVITA_DOM_OS_FILE_H_

#include "evita/dom/os/abstract_file.h"

#include "evita/dom/public/io_callback.h"

namespace domapi {
struct FileStatus;
}

namespace dom {

class MoveFileOptions;

namespace bindings {
class FileClass;
}

class File final : public ginx::Scriptable<File, AbstractFile> {
  DECLARE_SCRIPTABLE_OBJECT(File);

 public:
  explicit File(domapi::IoContextId context_id);
  ~File() final;

 private:
  friend class bindings::FileClass;

  static v8::Local<v8::Promise> MakeTempFileName(const base::string16& dir_name,
                                                 const base::string16& prefix);

  // Move |src_path| to |dst_path|
  static v8::Local<v8::Promise> Move(const base::string16& src_path,
                                     const base::string16& dst_path,
                                     const MoveFileOptions& options);
  static v8::Local<v8::Promise> Move(const base::string16& src_path,
                                     const base::string16& dst_path);

  static v8::Local<v8::Promise> Open(const base::string16& file_name,
                                     const base::string16& opt_mode);
  static v8::Local<v8::Promise> Open(const base::string16& file_name);

  // Remove a existing |file_name|
  static v8::Local<v8::Promise> Remove(const base::string16& file_name);

  static v8::Local<v8::Promise> Stat(const base::string16& file_name);

  DISALLOW_COPY_AND_ASSIGN(File);
};

}  // namespace dom

namespace gin {
template <>
struct Converter<domapi::FileId> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   domapi::FileId context_id);
};

template <>
struct Converter<domapi::MoveFileOptions> {
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     domapi::MoveFileOptions* out_options);
};
}  // namespace gin

#endif  // EVITA_DOM_OS_FILE_H_
