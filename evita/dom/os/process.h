// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE process.

#ifndef EVITA_DOM_OS_PROCESS_H_
#define EVITA_DOM_OS_PROCESS_H_

#include "evita/dom/os/abstract_file.h"

namespace dom {

namespace bindings {
class ProcessClass;
}

class Process final : public v8_glue::Scriptable<Process, AbstractFile> {
  DECLARE_SCRIPTABLE_OBJECT(Process);

 public:
  explicit Process(domapi::IoContextId context_id);
  ~Process() final;

 private:
  friend class bindings::ProcessClass;

  static v8::Local<v8::Promise> Open(const base::string16& command_line);

  DISALLOW_COPY_AND_ASSIGN(Process);
};

}  // namespace dom

#endif  // EVITA_DOM_OS_PROCESS_H_
