// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_editor_h)
#define INCLUDE_evita_dom_editor_h

#include <memory>

#include "base/strings/string16.h"
#include "base/synchronization/lock.h"
#include "common/memory/singleton.h"
#include "evita/v8_glue/script_wrappable.h"

namespace dom {

class Editor : public common::Singleton<Editor>,
               public v8_glue::ScriptWrappable<Editor> {
  friend class common::Singleton<Editor>;

  private: std::unique_ptr<base::Lock> lock_;

  private: Editor();
  public: virtual ~Editor() = default;

  public: base::Lock* lock() const { return lock_.get(); }
  public: static v8_glue::ScriptWrapperInfo* static_wrapper_info();
  public: const base::string16& version() const;

  // [G]
  public: virtual gin::ObjectTemplateBuilder
      GetObjectTemplateBuilder(v8::Isolate* isolate) override;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace dom

#define ASSERT_DOM_IS_LOCKED() \
  dom::Editor::instance().lock()->AssertAcquired()

#define DOM_AUTO_LOCK_SCOPE() \
  base::AutoLock dom_lock_scope(*dom::Editor::instance().lock());

#define DOM_AUTO_UNLOCK_SCOPE() \
  base::AutoUnlock dom_lock_scope(*dom::Editor::instance().lock());

#endif //!defined(INCLUDE_evita_dom_editor_h)
