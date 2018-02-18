// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE process.

#include "evita/dom/components/win_resource/win_resource.h"

#include <windows.h>

#include "evita/dom/os/io_error.h"
#include "evita/dom/promise_resolver.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/script_host.h"
#include "evita/ginx/array_buffer_view.h"

namespace gin {
template <>
struct Converter<domapi::WinResourceId> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   domapi::WinResourceId resource_id) {
    return gin::ConvertToV8(isolate, new dom::WinResource(resource_id));
  }
};
}  // namespace gin

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// WinResource
//
WinResource::WinResource(const domapi::WinResourceId& resource_id)
    : resource_id_(resource_id) {}

WinResource::~WinResource() {}

v8::Local<v8::Promise> WinResource::Close(ScriptHost* script_host) {
  return PromiseResolver::Call(
      FROM_HERE, base::BindOnce(&domapi::IoDelegate::CloseContext,
                                base::Unretained(script_host->io_delegate()),
                                resource_id_));
}

v8::Local<v8::Promise> WinResource::GetResourceNames(
    ScriptHost* script_host,
    const base::string16& type) {
  return PromiseResolver::Call(
      FROM_HERE, base::BindOnce(&domapi::IoDelegate::GetWinResourceNames,
                                base::Unretained(script_host->io_delegate()),
                                resource_id_, type));
}

v8::Local<v8::Promise> WinResource::Load(ScriptHost* script_host,
                                         const base::string16& type,
                                         const base::string16& name,
                                         const gin::ArrayBufferView& buffer) {
  return PromiseResolver::Call(
      FROM_HERE,
      base::BindOnce(&domapi::IoDelegate::LoadWinResource,
                     base::Unretained(script_host->io_delegate()), resource_id_,
                     type, name, static_cast<uint8_t*>(buffer.bytes()),
                     buffer.num_bytes()));
}

int WinResource::LookupIcon(const gin::ArrayBufferView& buffer, int icon_size) {
  return ::LookupIconIdFromDirectoryEx(static_cast<uint8_t*>(buffer.bytes()),
                                       true, icon_size, icon_size,
                                       LR_DEFAULTCOLOR);
}

// static
v8::Local<v8::Promise> WinResource::Open(ScriptHost* script_host,
                                         const base::string16& file_name) {
  return PromiseResolver::Call(
      FROM_HERE,
      base::BindOnce(&domapi::IoDelegate::OpenWinResource,
                     base::Unretained(script_host->io_delegate()), file_name));
}

}  // namespace dom
