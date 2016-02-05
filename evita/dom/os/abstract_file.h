// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_OS_ABSTRACT_FILE_H_
#define EVITA_DOM_OS_ABSTRACT_FILE_H_

#include "evita/dom/public/io_context_id.h"
#include "evita/dom/public/io_error.h"
#include "evita/ginx/scriptable.h"

namespace gin {
class ArrayBufferView;
}

namespace dom {

namespace bindings {
class AbstractFileClass;
}

class AbstractFile : public ginx::Scriptable<AbstractFile> {
  DECLARE_SCRIPTABLE_OBJECT(AbstractFile);

 protected:
  explicit AbstractFile(domapi::IoContextId context_id);
  ~AbstractFile() override;

 private:
  friend class bindings::AbstractFileClass;

  v8::Local<v8::Promise> Close();
  v8::Local<v8::Promise> Read(const gin::ArrayBufferView& array_buffer_view);
  v8::Local<v8::Promise> Write(const gin::ArrayBufferView& array_buffer_view);

  domapi::IoContextId context_id_;

  DISALLOW_COPY_AND_ASSIGN(AbstractFile);
};

}  // namespace dom

#endif  // EVITA_DOM_OS_ABSTRACT_FILE_H_
