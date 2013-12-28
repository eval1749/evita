// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_editor_dom_lock_h)
#define INCLUDE_evita_editor_dom_lock_h

#include "base/logging.h"
#pragma warning(push)
#pragma warning(disable: 4625)
#include "base/threading/thread_checker.h"
#pragma warning(pop)

namespace editor {

class DomLock {
  public: class AutoLock {
    public: AutoLock(const char* filename, int line_number);
    public: ~AutoLock();
    DISALLOW_COPY_AND_ASSIGN(AutoLock);
  };
  friend class AutoLock;

  public: class AutoTryLock {
    private: bool locked_;
    public: AutoTryLock(const char* filename, int line_number);
    public: ~AutoTryLock();
    public: bool locked() const { return locked_; }
    DISALLOW_COPY_AND_ASSIGN(AutoTryLock);
  };
  friend class AutoTryLock;

  public: class AutoUnlock {
    public: AutoUnlock(const char* filename, int line_number);
    public: ~AutoUnlock();
    DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
  };
  friend class AutoUnlock;

  private: const char* locker_filename_;
  private: int locker_line_number_;
  private: bool locked_;
  private: base::ThreadChecker thread_checker_;

  public: DomLock();
  public: ~DomLock() = default;

  public: static DomLock* instance();
  public: bool locked() const {
    DCHECK(thread_checker_.CalledOnValidThread());
    return locked_;
  }

  public: void Lock();
  public: bool TryLock();
  public: void Unlock();

  DISALLOW_COPY_AND_ASSIGN(DomLock);
};

}  // namespace editor

#define UI_ASSERT_DOM_LOCKED() \
  DCHECK(editor::DomLock::instance()->locked())

#define UI_DOM_AUTO_LOCK_SCOPE() \
  editor::DomLock::AutoLock dom_scope(__FILE__, __LINE__)

#define UI_DOM_AUTO_TRY_LOCK_SCOPE(scope_name) \
  editor::DomLock::AutoTryLock scope_name(__FILE__, __LINE__)

#define UI_DOM_AUTO_UNLOCK_SCOPE() \
  editor::DomLock::AutoUnlock dom_scope(__FILE__, __LINE__)

#endif //!defined(INCLUDE_evita_editor_dom_lock_h)
