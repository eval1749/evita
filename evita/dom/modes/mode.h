// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_mode_h)
#define INCLUDE_evita_dom_modes_mode_h

#include "base/strings/string16.h"
#include "evita/li_util.h"

namespace text {

class Buffer;
class Mode;

class ModeFactory;

class ModeFactory : public DoubleLinkedNode_<ModeFactory> {
  protected: const uint32_t* m_prgnCharSyntax;

  public: ModeFactory(const uint* prgnCharSyntax);
  public: virtual ~ModeFactory();

  // [C]
  public: virtual Mode* Create(Buffer*) = 0;

  // [E]
  protected: virtual const char16* getExtensions() const { return L""; }

  // [G]
  public: virtual const char16* GetName() const = 0;
  public: static ModeFactory* Get(Buffer*);
  public: uint32_t GetCharSyntax(char16) const;

  // [I]
  public: virtual bool IsSupported(const char16*) const;
};

typedef DoubleLinkedList_<ModeFactory> ModeFactoryes;

class Mode {
  protected: Buffer* m_pBuffer;
  protected: ModeFactory* m_pClass;

  protected: Mode(ModeFactory*, Buffer*);
  public: virtual ~Mode();

  // [D]
  public: virtual bool DoColor(Count) = 0;

  // [G]
  public: Buffer* GetBuffer() const { return m_pBuffer; }

  public: ModeFactory* GetClass() const { return m_pClass; }

  public: uint GetCharSyntax(char16 wch) const {
    return m_pClass->GetCharSyntax(wch);
  }

  public: virtual const char16* GetName() const {
    return m_pClass->GetName();
  }
};

} // namespace text

#endif //!defined(INCLUDE_evita_dom_modes_mode_h)
