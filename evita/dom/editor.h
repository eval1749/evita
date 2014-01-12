// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_editor_h)
#define INCLUDE_evita_dom_editor_h

#include "base/strings/string16.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class Editor : public v8_glue::Scriptable<Editor> {
  DECLARE_SCRIPTABLE_OBJECT(Editor)

  private: Editor();
  public: virtual ~Editor();

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_editor_h)
