// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_editor_application_h)
#define INCLUDE_evita_editor_application_h

#include <memory>

#pragma warning(push)
#pragma warning(disable: 4625)
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

namespace Command {
class Processor;
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
  protected: typedef DoubleLinkedList_<Buffer> Buffers;

  private: NewlineMode newline_mode_;
  private: uint code_page_;
  private: Buffers buffers_;
  private: Frames frames_;
  private: Frame* active_frame_;
  private: int idle_count_;
  private: bool is_quit_;
  private: std::unique_ptr<Command::Processor> command_processor_;
  private: std::unique_ptr<editor::DomLock> dom_lock_;
  private: std::unique_ptr<IoManager> io_manager_;
  private: std::unique_ptr<base::MessageLoop> message_loop_;
  private: std::unique_ptr<views::ViewDelegateImpl> view_delegate_impl_;

  // ctor/dtor
  friend class common::Singleton<Application>;
  private: Application();
  public: ~Application();

  public: const Buffers& buffers() const { return buffers_; }
  public: Buffers& buffers() { return buffers_; }
  public: const Frames& frames() const { return frames_; }
  public: Frames& frames() { return frames_; }
  public: editor::DomLock* dom_lock() const { return dom_lock_.get(); }
  public: const base::string16& title() const;
  public: const base::string16& version() const;
  public: dom::ViewEventHandler* view_event_handler() const;

  // [A]
  public: int Ask(int flags, int format_id, ...);

  // [C]
  public: bool CalledOnValidThread() const;

  // [D]
  public: void DidCreateFrame(Frame* frame);
  private: void DoIdle();

  // [E]
  public: void Execute(CommandWindow* window, uint32 key_code, uint32 repeat);

  // [F]
  public: Buffer* FindBuffer(const base::string16& name) const;
  public: Frame* FindFrame(HWND hwnd) const;
  public: Pane* FindPane(HWND hwnd, POINT point) const;

  // [G]
  public: Frame* GetActiveFrame() const { return active_frame_; }
  public: uint GetCodePage() const { return code_page_; }
  public: Frame* GetFirstFrame() const { return frames_.GetFirst(); }
  public: HIMAGELIST GetIconList() const;
  public: IoManager* GetIoManager() const { return io_manager_.get(); }
  public: Frame* GetLastFrame() const { return frames_.GetLast(); }
  public: NewlineMode GetNewline() const { return newline_mode_; }
  public: const char16* GetTitle() const;

  // [O]
  public: bool OnIdle(uint hint);

  // [P]
  public: void PostDomTask(const tracked_objects::Location& from_here,
                           const base::Closure& task);

  // [R]
  public: Buffer* RenameBuffer(Buffer* buffer,
                               const base::string16& new_name);
  public: void Run();

  // [S]
  public: bool SaveBuffer(Frame* frame, Buffer* buffer, bool save_as = false);

  public: Frame* SetActiveFrame(Frame* frame) { 
    return active_frame_ = frame;
  }

  public: void ShowMessage(MessageLevel, uint);

  // [T]
  private: bool TryDoIdle();

  // [W]
  public: void WillDestroyFrame(Frame* frame);

  DISALLOW_COPY_AND_ASSIGN(Application);
};

#define ASSERT_CALLED_ON_UI_THREAD() \
  DCHECK(Application::instance()->CalledOnValidThread())


#endif //!defined(INCLUDE_evita_editor_application_h)
