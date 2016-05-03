// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef EVITA_DOM_GLOBAL_H_
#define EVITA_DOM_GLOBAL_H_

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "common/memory/singleton.h"
#include "evita/ginx/scoped_persistent.h"

namespace ginx {
class Runner;
}

namespace dom {

class Global final : public common::Singleton<Global> {
 public:
  ~Global() final;

  v8::Local<v8::ObjectTemplate> GetObjectTemplate(v8::Isolate* isolate);
  static bool LoadGlobalScript(ginx::Runner* runner);
  static bool LoadModule(ginx::Runner* runner, base::StringPiece name);

 private:
  friend class common::Singleton<Global>;

  Global();

  ginx::ScopedPersistent<v8::ObjectTemplate> object_template_;

  DISALLOW_COPY_AND_ASSIGN(Global);
};

}  // namespace dom

#endif  // EVITA_DOM_GLOBAL_H_
