//////////////////////////////////////////////////////////////////////////////
//
// Editor - Edit Mode - Python
// listener/winapp/mode_Python.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/mode_Python.h#4 $
//
#if !defined(INCLUDE_mode_Python_h)
#define INCLUDE_mode_Python_h

#include "evita/text/modes/mode_factory.h"
#include "evita/text/buffer.h"

namespace text {

/// <summary>
///   PythonMode factory
/// </summary>
class PythonModeFactory : public ModeFactory {
  // ctor
  public: PythonModeFactory();

  // [C]
  public: virtual Mode* Create(Buffer*) override;

  // [G]
  protected: virtual const char16* getExtensions() const override {
    return L"py gyp gypi";
  }

  public: virtual const char16* GetName() const override { return L"Python"; }

  DISALLOW_COPY_AND_ASSIGN(PythonModeFactory);
}; // PythonModeFactory

}  // namespace text

#endif //!defined(INCLUDE_mode_Python_h)
