// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE process.

#if !defined(INCLUDE_evita_dom_os_process_h)
#define INCLUDE_evita_dom_os_process_h

#include "evita/dom/os/abstract_file.h"

namespace dom {

namespace bindings {
class ProcessClass;
}

class Process : public v8_glue::Scriptable<Process, AbstractFile> {
  DECLARE_SCRIPTABLE_OBJECT(Process);
  friend class bindings::ProcessClass;

  public: Process(domapi::IoContextId context_id);
  public: virtual ~Process();

  private: static v8::Handle<v8::Promise> Open(
      const base::string16& command_line);

  DISALLOW_COPY_AND_ASSIGN(Process);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_os_process_h)
