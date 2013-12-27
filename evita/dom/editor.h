// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_editor_h)
#define INCLUDE_evita_dom_editor_h

#include "evita/v8_glue/script_wrappable.h"

#include "base/strings/string16.h"

namespace dom {

class Editor : public v8_glue::ScriptWrappable<Editor> {
  public: Editor() = default;
  public: virtual ~Editor() = default;

  public: static v8_glue::ScriptWrapperInfo* static_wrapper_info();
  public: const base::string16& version() const;

  // [G]
  public: virtual gin::ObjectTemplateBuilder
      GetObjectTemplateBuilder(v8::Isolate* isolate) override;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_editor_h)
