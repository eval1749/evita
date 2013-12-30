// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_script_thread_h)
#define INCLUDE_evita_dom_script_thread_h

#include <memory>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/location.h"
#include "common/memory/singleton.h"

namespace base {
class Thread;
}

namespace dom {

class ScriptThread : public common::Singleton<ScriptThread> {
  friend class common::Singleton<ScriptThread>;

  private: std::unique_ptr<base::Thread> thread_;

  private: ScriptThread();
  public: ~ScriptThread();

  public: bool CalledOnValidThread() const;

  public: void PostTask(const tracked_objects::Location& from_here,
                        const base::Closure& task);

  DISALLOW_COPY_AND_ASSIGN(ScriptThread);
};

}  // namespace dom

#define ASSERT_CALLED_ON_SCRIPT_THREAD() \
  DCHECK(dom::ScriptThread::instance()->CalledOnValidThread())

#endif //!defined(INCLUDE_evita_dom_script_thread_h)
