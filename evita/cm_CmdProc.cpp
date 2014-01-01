#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - FontSet class
// listener/winapp/fontset.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/cm_CmdProc.cpp#2 $
//
#include "evita/cm_CmdProc.h"

#include "base/logging.h"
#include "base/strings/string16.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/ed_util.h"
#include "evita/dom/buffer.h"
#include "evita/vi_EditPane.h"
#include "evita/vi_Selection.h"
#include "evita/vi_TextEditWindow.h"

namespace Command {

KeyBindEntry* QuotedInsertEntry();
KeyBindEntry* StartArgumentEntry();

//////////////////////////////////////////////////////////////////////
//
// Command
//
Command::Command(CommandFn function) : function_(function) {
}

Command::Command() : function_(nullptr) {
}

Command* Command::AsCommand() {
  return this;
}

void Command::Execute(const Context* context) {
  UI_DOM_AUTO_LOCK_SCOPE();
  DCHECK(function_);
  function_(context);
}

//////////////////////////////////////////////////////////////////////
//
// Context
//
Context::Context()
    : m_pLastCommand(nullptr),
      m_pThisCommand(nullptr) {
}

//////////////////////////////////////////////////////////////////////
//
// KeyBindEntry
//
Command* KeyBindEntry::AsCommand() {
  return nullptr;
}

void KeyBinds::Bind(int key_code,
                    const common::scoped_refptr<KeyBindEntry>& entry) {
  key_bindings_[key_code] = entry;
}

void KeyBinds::Bind(int key_code, Command::CommandFn function) {
  key_bindings_[key_code] = new Command(function);
}

void KeyBindEntry::Execute(const Context*) {
  NOTREACHED();
}

KeyBindEntry* KeyBindEntry::MapKey(int) const {
  NOTREACHED();
  return nullptr;
}

//////////////////////////////////////////////////////////////////////
//
// KeyBinds
//
KeyBindEntry* KeyBinds::MapKey(int key_code) const {
  auto const it = key_bindings_.find(key_code);
  return it == key_bindings_.end() ? nullptr : it->second.get();
}

//////////////////////////////////////////////////////////////////////
//
// Processor
//
Processor::Processor() {
  Reset();
}

/// <summary>
/// Advance command processing state with specified keyboard event.
/// </summary>
/// <param name="fRepeat">True if keyboard repeat event.</param>
/// <param name="nKey">Key code.</param>
/// <param name="pWindow">A window object causing an event.</param>
void Processor::Execute(CommandWindow* window, int key_code, int fRepeat) {
  m_pWindow = window;
  m_wchLast = static_cast<char16>(key_code);
  key_codes_.push_back(key_code);
  PrepareExecution(window);
  common::scoped_refptr<KeyBindEntry> entry;
  switch (m_eState) {
    case State_Arg:
      if (key_code >= '0' && key_code <= '9') {
        m_iArg *= 10;
        m_iArg += key_code - '0';
        return;
      }
      m_iArg *= m_iSign;
      // FALLTHROUGH

    case State_Continue:
      entry = last_entry_ ? last_entry_->MapKey(key_code) :
          window->MapKey(key_code);
      break;

    case State_Quote:
      if (m_pSelection) {
        // Insert ASCII character into buffer.
        if (key_code >= (Mod_Ctrl | 'A') && key_code <= (Mod_Ctrl | 0x5F))
          key_code &= 0x1F;

        if (key_code <= 0x7F)
          m_pSelection->TypeChar(static_cast<base::char16>(key_code));
      }
      Reset();
      return;

    case State_StartArg:
      if (key_code == '-') {
        m_eState = State_Arg;
        m_iSign = -1;
        return;
      }
      if (key_code >= '0' && key_code <= '9') {
        m_eState = State_Arg;
        m_iArg = static_cast<Count>(key_code - '0');
        return;
      }
      m_iArg = 4;
      entry = window->MapKey(key_code);
      break;

    case State_Start:
      m_pFrame->ResetMessages();
      entry = window->MapKey(key_code);
      m_iArg = fRepeat ? 2 : 1;
      if (entry == StartArgumentEntry()) {
        m_eState = State_StartArg;
        m_iArg = 0;
        m_iSign = 1;
        return;
      }
      break;

    default:
      NOTREACHED();
  }

  if (entry == StartArgumentEntry()) {
    m_eState = State_Continue;
    m_iArg *= 4;
    return;
  }

  if (entry == QuotedInsertEntry()) {
    m_eState = State_Quote;
    return;
  }

  if (!entry) {
    ReportUnboundKeys();
    Reset();
    return;
  }

  if (auto const command = entry->AsCommand()) {
    m_pThisCommand = command;
    entry->Execute(this);
    m_pLastCommand = m_pThisCommand;
    Reset();
  } else {
    last_entry_ = entry;
    m_eState = State_Continue;
  }
}

void Processor::PrepareExecution(CommandWindow* window) {
  auto const pEditWindow = window->as<TextEditWindow>();
  if (!pEditWindow) {
    m_pFrame = Application::instance()->GetActiveFrame();
    m_pSelection = nullptr;
    return;
  }
  m_pSelection = pEditWindow->GetSelection();
  m_pFrame = m_pSelection->GetWindow()->parent_node()->
      as<EditPane>()->GetFrame();
}

void Processor::ReportUnboundKeys() {
  base::string16 text;
  for (const auto key_code : key_codes_) {
    if (key_code & Mod_Ctrl)
      text.append(L"Ctrl+");

    if (key_code & Mod_Shift)
      text.append(L"Shift+");

    if (key_code <= 0xFF)
      text.push_back(static_cast<base::char16>(key_code));
    else
      text.append(GetKeyName(key_code));
  }

  Application::instance()->GetActiveFrame()->ShowMessage(
      MessageLevel_Warning,
      IDS_NOT_COMMAND,
      text.c_str());
}

void Processor::Reset() {
  m_eState = State_Start;
  m_iArg = 0;
  m_iSign = 0;
  key_codes_.resize(0);
  last_entry_ = nullptr;
}

}  // namespace Command
