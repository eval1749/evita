//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/cm_CmdProc.h#1 $
//
#if !defined(INCLUDE_listener_winapp_command_processor_h)
#define INCLUDE_listener_winapp_command_processor_h

#include "./li_util.h"

class CommandWindow;
class Frame;
class Selection;

enum Mod
{
    Mod_None        = 0,
    Mod_Ctrl        = 0x200,
    Mod_Shift       = 0x400,
    Mod_CtrlShift   = 0x600,
}; // Mod


namespace Command
{

enum Bind
{
    Bind_Command,
    Bind_KeyBinds,
    Bind_Argument,
}; // Bind

//////////////////////////////////////////////////////////////////////
//
// Context
//
class Context
{
    protected: enum State
    {
        State_Arg,
        State_Continue,
        State_Quote,
        State_Start,
        State_StartArg,
    }; // State

    protected: State            m_eState;
    protected: Count            m_iArg;
    protected: Count            m_iSign;
    protected: uint             m_cKeys;
    protected: uint             m_rgnKey[10];
    protected: CommandWindow*   m_pWindow;
    protected: Frame*     m_pFrame;
    protected: Selection*       m_pSelection;
    protected: char16               m_wchLast;

    // ctor
    public: Context() {}

    // [G]
    public: Count          GetArg()       const { return m_iArg; }
    public: Frame*   GetFrame()     const { return m_pFrame; }
    public: char16         GetLastChar()  const { return m_wchLast; }
    public: Selection*     GetSelection() const { return m_pSelection; }
    public: CommandWindow* GetWindow()    const { return m_pWindow; }

    // [H]
    public: bool HasArg() const
        { return 0 != m_iSign; }
}; // Context


class Command;
class KeyBinds;

//////////////////////////////////////////////////////////////////////
//
// KeyBindEntry
//
class KeyBindEntry : public Castable_<KeyBindEntry>
{
    public: virtual Bind            GetKind() const = 0;
}; // KeyBindEntry


//////////////////////////////////////////////////////////////////////
//
// Command
//
class Command : public HasKind_<Command, KeyBindEntry, Bind>
{
    public: static Bind Kind_() { return Bind_Command; }

    public: typedef void (*CommandFn)(const Context*);
    private: CommandFn  m_pfn;
    public: Command(CommandFn pfn) : m_pfn(pfn) {}
    public: void Execute(const Context* pCtx) { m_pfn(pCtx); }
}; // Command


//////////////////////////////////////////////////////////////////////
//
// KeyBinds
//
class KeyBinds : public HasKind_<KeyBinds, KeyBindEntry, Bind>
{
    public: static Bind Kind_() { return Bind_KeyBinds; }

    private: KeyBindEntry* m_rgpBind[0x100 * 8];

    public: KeyBinds();
    public: void Bind(uint, KeyBindEntry*);
    public: void Bind(uint, Command::CommandFn);
    public: KeyBindEntry* MapKey(uint) const;
}; // KeyBinds


//////////////////////////////////////////////////////////////////////
//
// Processor
//
class Processor : public Context
{
    private: static KeyBinds* sm_pGlobakBinds;

    private: Command*   m_pLastCommand;
    private: KeyBinds*  m_pKeyBinds;
    private: Command*   m_pThisCommand;

    // ctor
    public: Processor() :
        m_pLastCommand(NULL),
        m_pThisCommand(NULL)
            { Reset(); }

    // [E]
    public: void Execute(CommandWindow*, uint, uint);

    // [G]
    public: Command* GetLastCommand() const { return m_pLastCommand; }
    public: Command* GetThisCommand() const { return m_pThisCommand; }
    public: static void GlobalInit();

    // [R]
    protected: void reportUnboundKeys();
    public: void    Reset();
}; // Processor

extern KeyBinds* g_pGlobalBinds;

void TypeChar(const Context*);

inline uint MapVKey(uint nVKey)
{
    return nVKey | 0x100;
} // MapVKey

const char16* GetKeyName(uint);


#define DEFCOMMAND(mp_name) \
    void mp_name(const Context* pCtx)

} //Command

#endif //!defined(INCLUDE_listener_winapp_command_processor_h)
