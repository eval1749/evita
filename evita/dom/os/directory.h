// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_OS_DIRECTORY_H_
#define EVITA_DOM_OS_DIRECTORY_H_

#include "evita/dom/public/io_context_id.h"
#include "evita/dom/public/io_error.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

namespace bindings {
class DirectoryClass;
}

//////////////////////////////////////////////////////////////////////
//
// Directory
//
class Directory final : public v8_glue::Scriptable<Directory> {
  DECLARE_SCRIPTABLE_OBJECT(File);

 public:
  explicit Directory(domapi::IoContextId context_id);
  ~Directory() final;

 private:
  friend class bindings::DirectoryClass;

  v8::Local<v8::Promise> Close();
  static v8::Local<v8::Promise> Open(const base::string16& dir_name);
  v8::Local<v8::Promise> Read(int num_read);

  domapi::IoContextId context_id_;

  DISALLOW_COPY_AND_ASSIGN(Directory);
};

}  // namespace dom

#endif  // EVITA_DOM_OS_DIRECTORY_H_
