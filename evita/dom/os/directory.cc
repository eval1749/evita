// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/os/directory.h"

#include "evita/dom/os/file_status.h"
#include "evita/dom/os/io_error.h"
#include "evita/dom/promise_resolver.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/script_host.h"

namespace gin {
template <>
struct Converter<domapi::DirectoryId> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   domapi::DirectoryId context_id) {
    return gin::ConvertToV8(isolate, new dom::Directory(context_id));
  }
};
}  // namespace gin

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Directory
//
Directory::Directory(domapi::IoContextId context_id)
    : context_id_(context_id) {}

Directory::~Directory() {
  Close();
}

v8::Local<v8::Promise> Directory::Close() {
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::IoDelegate::CloseDirectory,
                 base::Unretained(ScriptHost::instance()->io_delegate()),
                 context_id_));
}

v8::Local<v8::Promise> Directory::Open(const base::string16& root_path) {
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::IoDelegate::OpenDirectory,
                 base::Unretained(ScriptHost::instance()->io_delegate()),
                 root_path));
}

v8::Local<v8::Promise> Directory::Read(int num_read) {
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::IoDelegate::ReadDirectory,
                 base::Unretained(ScriptHost::instance()->io_delegate()),
                 context_id_, num_read));
}

}  // namespace dom
