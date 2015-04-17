// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_global_h)
#define INCLUDE_evita_dom_global_h

#include "base/macros.h"
#include "common/memory/singleton.h"

#include "evita/v8_glue/scoped_persistent.h"

namespace dom {

class Global : public common::Singleton<Global> {
  friend class common::Singleton<Global>;

  private: v8_glue::ScopedPersistent<v8::ObjectTemplate> object_template_;

  private: Global();
  public: ~Global();

  public: v8::Handle<v8::ObjectTemplate>
      object_template(v8::Isolate* isolate);

  DISALLOW_COPY_AND_ASSIGN(Global);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_global_h)
