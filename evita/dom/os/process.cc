// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE process.

#include "evita/dom/os/process.h"

#include "evita/dom/promise_resolver.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/script_host.h"

namespace gin {
template <>
struct Converter<domapi::ProcessId> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    domapi::ProcessId context_id) {
    return gin::ConvertToV8(isolate, new dom::Process(context_id));
  }
};
}  // namespace gin

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Process
//
Process::Process(domapi::IoContextId context_id) : ScriptableBase(context_id) {}

Process::~Process() {}

v8::Handle<v8::Promise> Process::Open(const base::string16& command_line) {
  return PromiseResolver::Call(
      FROM_HERE,
      base::Bind(&domapi::IoDelegate::OpenProcess,
                 base::Unretained(ScriptHost::instance()->io_delegate()),
                 command_line));
}

}  // namespace dom
