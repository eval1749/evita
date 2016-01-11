// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_GLOBAL_H_
#define EVITA_DOM_GLOBAL_H_

#include "base/macros.h"
#include "common/memory/singleton.h"

#include "evita/v8_glue/scoped_persistent.h"

namespace dom {

class Global final : public common::Singleton<Global> {
 public:
  ~Global() final;

  v8::Local<v8::ObjectTemplate> object_template(v8::Isolate* isolate);

 private:
  friend class common::Singleton<Global>;

  Global();

  v8_glue::ScopedPersistent<v8::ObjectTemplate> object_template_;

  DISALLOW_COPY_AND_ASSIGN(Global);
};

}  // namespace dom

#endif  // EVITA_DOM_GLOBAL_H_
