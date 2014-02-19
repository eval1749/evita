// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_editor_application_h)
#define INCLUDE_evita_editor_application_h

#include <memory>

// L4 C4625: 'derived class' : copy constructor could not be generated because
// a base class copy constructor is inaccessible
// L4 C4626: 'derived class' : assignment operator could not be generated
// because a base class assignment operator is inaccessible
#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/callback.h"
#pragma warning(pop)
#include "base/location.h"
#include "base/strings/string16.h"
#include "common/memory/singleton.h"
#include "evita/vi_Frame.h"

class CommandWindow;
class IoManager;

namespace base {
class MessageLoop;
}

namespace dom {
class Buffer;
class ViewEventHandler;
}

using Buffer = dom::Buffer;

namespace editor {
class DomLock;
}

namespace views {
class ViewDelegateImpl;
}

class Application : public common::Singleton<Application> {
  protected: typedef DoubleLinkedList_<Frame> Frames;

  private: Frames frames_;
  private: Frame* active_frame_;
  private: int idle_count_;
  private: bool is_quit_;
  private: std::unique_ptr<editor::DomLock> dom_lock_;
  private: std::unique_ptr<IoManager> io_manager_;
  private: std::unique_ptr<base::MessageLoop> message_loop_;
  private: std::unique_ptr<views::ViewDelegateImpl> view_delegate_impl_;

  // ctor/dtor
  friend class common::Singleton<Application>;
  private: Application();
  public: ~Application();

  public: const Frames& frames() const { return frames_; }
  public: Frames& frames() { return frames_; }
  public: editor::DomLock* dom_lock() const { return dom_lock_.get(); }
  public: const base::string16& title() const;
  public: const base::string16& version() const;
  public: dom::ViewEventHandler* view_event_handler() const;

  // [C]
  public: bool CalledOnValidThread() const;

  // [D]
  public: void DidCreateFrame(Frame* frame);
  private: void DoIdle();

  // [F]
  public: Frame* FindFrame(HWND hwnd) const;

  // [G]
  public: Frame* GetActiveFrame() const { return active_frame_; }
  public: IoManager* GetIoManager() const { return io_manager_.get(); }
  public: const char16* GetTitle() const;

  // [O]
  public: bool OnIdle(uint hint);

  // [P]
  public: void PostDomTask(const tracked_objects::Location& from_here,
                           const base::Closure& task);

  // [R]
  public: void Run();

  // [S]
  public: Frame* SetActiveFrame(Frame* frame) { 
    return active_frame_ = frame;
  }

  // [T]
  private: bool TryDoIdle();

  // [W]
  public: void WillDestroyFrame(Frame* frame);

  DISALLOW_COPY_AND_ASSIGN(Application);
};

#define ASSERT_CALLED_ON_UI_THREAD() \
  DCHECK(Application::instance()->CalledOnValidThread())


#endif //!defined(INCLUDE_evita_editor_application_h)
