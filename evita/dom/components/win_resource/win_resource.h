// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_COMPONENTS_WIN_RESOURCE_WIN_RESOURCE_H_
#define EVITA_DOM_COMPONENTS_WIN_RESOURCE_WIN_RESOURCE_H_

#include "evita/dom/public/io_context_id.h"
#include "evita/ginx/scriptable.h"

namespace gin {
class ArrayBufferView;
}

namespace dom {

class ScriptHost;

namespace bindings {
class WinResourceClass;
}

//////////////////////////////////////////////////////////////////////
//
// WinResource
//
class WinResource final : public ginx::Scriptable<WinResource> {
  DECLARE_SCRIPTABLE_OBJECT(WinResource);

 public:
  explicit WinResource(const domapi::WinResourceId& resource_id);
  ~WinResource() final;

 private:
  friend class bindings::WinResourceClass;

  // bindings
  v8::Local<v8::Promise> Close(ScriptHost* script_host);
  v8::Local<v8::Promise> GetResourceNames(ScriptHost* script_host,
                                          const base::string16& type);
  v8::Local<v8::Promise> Load(ScriptHost* script_host,
                              const base::string16& type,
                              const base::string16& name,
                              const gin::ArrayBufferView& buffer);
  static int LookupIcon(const gin::ArrayBufferView& buffer, int icon_size);
  static v8::Local<v8::Promise> Open(ScriptHost* script_host,
                                     const base::string16& file_name);

  domapi::WinResourceId resource_id_;

  DISALLOW_COPY_AND_ASSIGN(WinResource);
};

}  // namespace dom

#endif  // EVITA_DOM_COMPONENTS_WIN_RESOURCE_WIN_RESOURCE_H_
