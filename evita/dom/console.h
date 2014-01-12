// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_console_h)
#define INCLUDE_evita_dom_console_h

#include <vector>

#include "base/strings/string16.h"
#include "common/memory/singleton.h"
#include "evita/v8_glue/scriptable.h"
#include "evita/view/window_id.h"

namespace dom {

class Console : public common::Singleton<Console>,
                public v8_glue::Scriptable<Console> {
  DECLARE_SCRIPTABLE_OBJECT(Console);

  friend class common::Singleton<Console>;

  private: Console();
  public: virtual ~Console();

  DISALLOW_COPY_AND_ASSIGN(Console);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_console_h)
