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
#if !defined(INCLUDE_evita_text_modes_python_mode_h)
#define INCLUDE_evita_text_modes_python_mode_h

#include "common/memory/singleton.h"
#include "evita/text/modes/mode.h"
#include "evita/text/modes/mode_factory.h"

namespace text {

/// <summary>
///   PythonMode factory
/// </summary>
class PythonModeFactory : public common::Singleton<PythonModeFactory>,
                          public ModeFactory {
  DECLARE_SINGLETON_CLASS(PythonModeFactory);

  // ctor
  private: PythonModeFactory();
  public: ~PythonModeFactory();

  // [C]
  public: virtual Mode* Create() override;

  // [G]
  protected: virtual const char16* getExtensions() const override {
    return L"py gyp gypi";
  }

  public: virtual const char16* GetName() const override { return L"Python"; }

  DISALLOW_COPY_AND_ASSIGN(PythonModeFactory);
}; // PythonModeFactory

class PythonMode : public ModeWithLexer {
  public: PythonMode();
  public: virtual ~PythonMode();

  // Mode
  private: virtual const char16* GetName() const override;

  // ModeWithLexer
  private: virtual Lexer* CreateLexer(Buffer* buffer) override;

  DISALLOW_COPY_AND_ASSIGN(PythonMode);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_modes_python_mode_h)
