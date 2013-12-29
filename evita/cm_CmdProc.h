// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_listener_winapp_command_processor_h)
#define INCLUDE_listener_winapp_command_processor_h

#include <unordered_map>
#include <vector>

#include "common/memory/ref_counted.h"
#include "common/memory/scoped_refptr.h"
#include "evita/li_util.h"

class CommandWindow;
class Frame;
class Selection;

enum Mod {
  Mod_None = 0,
  Mod_Ctrl = 0x200,
  Mod_Shift = 0x400,
  Mod_CtrlShift = 0x600,
};

namespace Command {

enum Bind {
  Bind_Command,
  Bind_KeyBinds,
  Bind_Argument,
};

class Command;
class KeyBinds;
class Processor;

//////////////////////////////////////////////////////////////////////
//
// Context
//
class Context {
  friend class Processor;

  protected: enum State {
    State_Arg,
    State_Continue,
    State_Quote,
    State_Start,
    State_StartArg,
  };

  protected: State m_eState;
  protected: Count m_iArg;
  protected: Count m_iSign;
  protected: std::vector<int> key_codes_;
  private: common::scoped_refptr<Command> m_pLastCommand;
  private: common::scoped_refptr<Command> m_pThisCommand;
  protected: CommandWindow* m_pWindow;
  protected: Frame* m_pFrame;
  protected: Selection* m_pSelection;
  protected: char16 m_wchLast;

  // ctor
  public: Context();
  public: ~Context() = default;

  // [G]
  public: Count GetArg() const { return m_iArg; }
  public: Frame* GetFrame() const { return m_pFrame; }
  public: char16 GetLastChar() const { return m_wchLast; }
  public: Command* GetLastCommand() const { return m_pLastCommand.get(); }
  public: Selection* GetSelection() const { return m_pSelection; }
  public: Command* GetThisCommand() const { return m_pThisCommand.get(); }
  public: CommandWindow* GetWindow() const { return m_pWindow; }

  // [H]
  public: bool HasArg() const { return 0 != m_iSign; }

  DISALLOW_COPY_AND_ASSIGN(Context);
};

//////////////////////////////////////////////////////////////////////
//
// KeyBindEntry
//
class KeyBindEntry : public common::RefCounted<KeyBindEntry> {
  protected: KeyBindEntry() = default;
  public: virtual ~KeyBindEntry() = default;
  public: virtual Command* AsCommand();
  public: virtual void Execute(const Context* context);
  public: virtual KeyBindEntry* MapKey(int key_code) const;
  DISALLOW_COPY_AND_ASSIGN(KeyBindEntry);
};

//////////////////////////////////////////////////////////////////////
//
// Command
//
class Command : public KeyBindEntry {
  public: typedef void (*CommandFn)(const Context*);

  private: CommandFn function_;

  public: Command(CommandFn function);
  protected: Command();
  public: virtual ~Command() = default;

  public: virtual Command* AsCommand() override;
  public: virtual void Execute(const Context* context) override;

  DISALLOW_COPY_AND_ASSIGN(Command);
};

//////////////////////////////////////////////////////////////////////
//
// KeyBinds
//
class KeyBinds : public KeyBindEntry {
  private: std::unordered_map<int, common::scoped_refptr<KeyBindEntry>>
      key_bindings_;

  public: KeyBinds() = default;
  public: virtual ~KeyBinds() = default;

  public: void Bind(int key_code,
                    const common::scoped_refptr<KeyBindEntry>& entry);
  public: void Bind(int key_code, Command::CommandFn);
  public: virtual KeyBindEntry* MapKey(int) const override;

  DISALLOW_COPY_AND_ASSIGN(KeyBinds);
};

//////////////////////////////////////////////////////////////////////
//
// Processor
//
class Processor : public Context {
  private: common::scoped_refptr<KeyBindEntry> last_entry_;

  // ctor
  public: Processor();
  public: virtual ~Processor() = default;

  public: void Execute(CommandWindow* window, int key_code, int repeat);
  public: static void GlobalInit();
  private: void PrepareExecution(CommandWindow* window);
  private: void ReportUnboundKeys();
  public: void Reset();

  DISALLOW_COPY_AND_ASSIGN(Processor);
};

extern KeyBinds* g_pGlobalBinds;

void TypeChar(const Context*);

inline uint MapVKey(uint nVKey) {
  return nVKey | 0x100;
}

const char16* GetKeyName(uint);

#define DEFCOMMAND(mp_name) \
  void mp_name(const Context* pCtx)

}  // namespace Command

#endif //!defined(INCLUDE_listener_winapp_command_processor_h)
