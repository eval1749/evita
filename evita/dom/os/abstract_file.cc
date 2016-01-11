// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/os/abstract_file.h"

#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "evita/dom/converter.h"
#include "evita/dom/os/io_error.h"
#include "evita/dom/promise_resolver.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/public/io_error.h"
#include "evita/dom/script_host.h"
#include "evita/v8_glue/runner.h"
#include "gin/array_buffer.h"
#include "v8_strings.h"  // NOLINT(build/include)

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// File
//
AbstractFile::AbstractFile(domapi::IoContextId context_id)
    : context_id_(context_id) {}

AbstractFile::~AbstractFile() {
  Close();
}

v8::Local<v8::Promise> AbstractFile::Close() {
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::IoDelegate::CloseFile,
                 base::Unretained(ScriptHost::instance()->io_delegate()),
                 context_id_));
}

v8::Local<v8::Promise> AbstractFile::Read(
    const gin::ArrayBufferView& array_buffer_view) {
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::IoDelegate::ReadFile,
                 base::Unretained(ScriptHost::instance()->io_delegate()),
                 context_id_, array_buffer_view.bytes(),
                 array_buffer_view.num_bytes()));
}

v8::Local<v8::Promise> AbstractFile::Write(
    const gin::ArrayBufferView& array_buffer_view) {
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::IoDelegate::WriteFile,
                 base::Unretained(ScriptHost::instance()->io_delegate()),
                 context_id_, array_buffer_view.bytes(),
                 array_buffer_view.num_bytes()));
}

}  // namespace dom
