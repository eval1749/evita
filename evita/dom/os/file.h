// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_os_file_h)
#define INCLUDE_evita_dom_os_file_h

#include "evita/dom/os/abstract_file.h"

namespace dom {
namespace os {

class File : public v8_glue::Scriptable<File, AbstractFile> {
  DECLARE_SCRIPTABLE_OBJECT(File);

  public: File(domapi::IoContextId context_id);
  public: virtual ~File();

  DISALLOW_COPY_AND_ASSIGN(File);
};

}  // namespace os
}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_os_file_h)
