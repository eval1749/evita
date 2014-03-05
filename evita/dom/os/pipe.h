// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_os_pipe_h)
#define INCLUDE_evita_dom_os_pipe_h

#include "evita/v8_glue/scriptable.h"

namespace dom {

class Pipe : public v8_glue::Scriptable<Pipe> {
  private: gc::Member<FileHandle> input_handle_;
  private: gc::Member<FileHandle> output_handle_;

  public: Pipe();
  public: virtual ~Pipe();

  public: FileHandle* input() const { return input_handle_.get(); }
  public: FileHandle* output() const { return output_handle_.get(); }
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_os_pipe_h)
