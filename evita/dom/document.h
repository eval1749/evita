//////////////////////////////////////////////////////////////////////////////
//
// Editor - Buffer
// listener/winapp/ed_Buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Buffer.h#1 $
//
#if !defined(INCLUDE_evita_dom_document_h)
#define INCLUDE_evita_dom_document_h

#include "evita/core/buffer.h"

#include "./vi_TextEditWindow.h"

namespace Command {
class KeyBindEntry;
}

namespace dom {

class Buffer : public Edit::Buffer, public DoubleLinkedNode_<Buffer> {
  public: typedef TextEditWindow Window;
  private: WindowList m_oWindows;

  public: Buffer(const char16* pwsz, Edit::Mode* pMode = nullptr);
  public: virtual ~Buffer();

  public: const WindowList& windows() const { return m_oWindows; }
  public: WindowList& windows() { return m_oWindows; }

  // [A]
  public: void AddWindow(Window* pWindow) { m_oWindows.Append(pWindow); }

  // [C]
  public: bool CanKill();

  // [F]
  public: void FinishIo(uint);

  // [G]
  public: Window* GetWindow() const;

  // [L]
  public: bool Load(const char16*);

  // [M]
  public: virtual Command::KeyBindEntry* MapKey(uint) const;

  // [O]
  public: bool OnIdle(uint);

  // [R]
  public: bool Reload();

  public: void RemoveWindow(Window* pWindow) { m_oWindows.Delete(pWindow); }

  // [S]
  public: bool Save(const char16*, uint, NewlineMode);

  // [U]
  public: void UpdateFileStatus(bool = false);
};

} // namespace dom

using Buffer = dom::Buffer;

#endif //!defined(INCLUDE_evita_dom_document_h)
