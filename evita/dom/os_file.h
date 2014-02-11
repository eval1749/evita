// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_os_file_h)
#define INCLUDE_evita_dom_os_file_h

#include "evita/v8_glue/v8.h"

namespace dom {

namespace os {
namespace file {
v8::Handle<v8::ObjectTemplate> CreateObjectTemplate(v8::Isolate* isolate);
}  // namespace file
}  // namespace os

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_os_file_h)
